#pragma once
#ifdef PS3_GAME_ORBIT_FIX41
#include "file_manager_fix41.h"
using FileManagerFix40=FileManagerFix41;
#else
#ifdef PS3_GAME_ORBIT_FIX40
#include "input_state.h"
#include <string>
#include <vector>
#include <cstdint>
class FileManagerFix40 {
public:
    enum class Mode { Notice, Browse, Overwrite, Copy };
    struct Entry {std::string name;bool directory=false,regular=false;std::uint64_t size=0;};
    struct View {bool active=false;Mode mode=Mode::Notice;std::string path,clipboard,status,destination;std::vector<Entry> entries;int selected=0;std::uint64_t done=0,total=0,revision=0;};
    explicit FileManagerFix40(std::string root="/dev_hdd0");
    ~FileManagerFix40();
    void open();void close();void update(const InputFrame& input,float dt);void step();
    bool active() const{return view_.active;}bool copying() const{return view_.mode==Mode::Copy;}
    const View& view() const{return view_;}
private:
    struct Snapshot {bool exists=false,regular=false,directory=false;std::uint64_t size=0;std::int64_t mtime=0,ctime=0;bool operator==(const Snapshot& o) const;};
    std::string root_,source_,temporary_,backup_;
    View view_;std::vector<unsigned char> buffer_;int source_fd_=-1,dest_fd_=-1;
    bool owns_temp_=false;unsigned serial_=0;float repeat_=0;int repeat_direction_=0;
    Snapshot source_snapshot_,dest_snapshot_;
    static Snapshot snapshot(const std::string& path,int fd=-1);
    void changed(){++view_.revision;}
    void refresh();void paste();void begin_copy();void cancel(const std::string& status);
    std::string unique(const char* role);void finish();
};
#endif
#endif
