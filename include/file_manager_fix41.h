#pragma once
#ifdef PS3_GAME_ORBIT_FIX41
#include "input_state.h"
#include <array>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
class FileManagerFix41 {
public:
    enum class Mode { Notice, Browse, Overwrite, Copy };
    struct Entry {std::string name;bool directory=false,regular=false;std::uint64_t size=0;bool marked=false,copied=false;};
    struct Device {std::string path,label;bool usb=false;};
    struct Panel {std::string path;std::vector<Entry> entries;int selected=0;};
    struct View {
        bool active=false;Mode mode=Mode::Notice;std::array<Panel,2> panels;int active_panel=0;
        std::string status,source,destination;std::uint64_t done=0,total=0,batch_done=0,batch_total=0,revision=0;
        unsigned file_index=0,file_count=0,copied_count=0,skipped_count=0,marked_count=0;int overwrite_selected=0;
    };
    explicit FileManagerFix41(std::string root="/dev_hdd0");
    explicit FileManagerFix41(std::vector<std::string> roots);
    ~FileManagerFix41();
    void open();void close();void update(const InputFrame& input,float dt);void step();
    bool active() const{return view_.active;}bool copying() const{return batch_active_;}
    const View& view() const{return view_;}
    const std::vector<Device>& devices() const{return devices_;}
private:
    struct Snapshot {
        bool exists=false,regular=false,directory=false;std::uint64_t size=0;std::int64_t mtime=0,ctime=0;
        bool operator==(const Snapshot& other) const;
    };
    struct Job {std::string source,destination;Snapshot source_snapshot;};
    std::vector<std::string> roots_;bool discover_usb_=false;std::vector<Device> devices_;
    View view_;std::unordered_set<std::string> marked_,copied_;std::vector<Job> jobs_;std::size_t job_=0;
    std::vector<unsigned char> buffer_;std::string temporary_,backup_;int source_fd_=-1,dest_fd_=-1;
    bool owns_temp_=false,batch_active_=false;unsigned serial_=0;int overwrite_all_=0;
    float repeat_=0;int repeat_direction_=0;Snapshot dest_snapshot_,source_fd_snapshot_;
    static Snapshot snapshot(const std::string& path,int fd=-1,int* error=nullptr);
    void changed(){++view_.revision;}
    void discover();void refresh(int panel,bool keep_selection=false);void flags();
    const Device* device_for(const std::string& path) const;
    void start_batch();void next();void begin_copy();void finish();void skip();
    void release_copy();void cancel(const std::string& status);void failure(const char* action,int error);
    std::string unique(const char* role);void batch_finished();
};
#endif
