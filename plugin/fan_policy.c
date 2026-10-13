#include "../include/fan_policy_fix48.h"

static uint32_t minimum(uint32_t a,uint32_t b){return a<b?a:b;}
static uint32_t maximum(uint32_t a,uint32_t b){return a>b?a:b;}
void orbit48_fan_init(Orbit48FanPolicy* p) {
    p->config=(Orbit48FanConfig){ORBIT48_FAN_SYSCON,68,35,85,40};
    p->percent=35;p->failures=0;p->cool_ticks=0;p->state=ORBIT48_FAN_READY;
}
int orbit48_fan_configure(Orbit48FanPolicy* p,const Orbit48FanConfig* c) {
    if(!orbit48_valid_fan_config(c))return 0;
    p->config=*c;p->percent=maximum(p->percent,c->minimum_percent);
    p->percent=minimum(p->percent,c->maximum_percent);
    p->failures=0;p->cool_ticks=0;return 1;
}
int orbit48_fan_tick(Orbit48FanPolicy* p,int cpu,int rsx,int ok,int external,int pause) {
    if(external){p->state=ORBIT48_FAN_EXTERNAL;return -2;}
    if(p->config.mode==ORBIT48_FAN_SYSCON){p->state=ORBIT48_FAN_READY;return -1;}
    if(!ok || cpu<5 || cpu>120 || rsx<5 || rsx>120) {
        p->state=ORBIT48_FAN_SENSOR_FAILURE;
        if(++p->failures>=3)return -1;
        return (int)p->percent;
    }
    p->failures=0;
    const int hottest=cpu>rsx?cpu:rsx;
    if(hottest>=83){p->percent=100;p->state=ORBIT48_FAN_EMERGENCY;return 100;}
    if(hottest>=80){p->percent=maximum(p->percent,85);p->state=ORBIT48_FAN_EMERGENCY;return (int)p->percent;}
    if(pause){p->state=ORBIT48_FAN_STARTING_GAME;return -2;}
    p->state=ORBIT48_FAN_READY;
    if(p->config.mode==ORBIT48_FAN_MANUAL){p->percent=p->config.manual_percent;return (int)p->percent;}
    if(hottest>=(int)p->config.target_c) {
        const uint32_t step=hottest>(int)p->config.target_c+4?5:2;
        p->percent=minimum(p->config.maximum_percent,p->percent+step);p->cool_ticks=0;
    }else if(hottest<=(int)p->config.target_c-2) {
        if(++p->cool_ticks>=5){if(p->percent>p->config.minimum_percent)--p->percent;p->cool_ticks=0;}
    }else p->cool_ticks=0;
    p->percent=maximum(p->config.minimum_percent,minimum(p->config.maximum_percent,p->percent));
    return (int)p->percent;
}
