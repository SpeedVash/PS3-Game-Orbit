#pragma once
#ifdef PS3_GAME_ORBIT_FIX42
#include "input_state.h"
#include <array>
#include <string>
#include <vector>
#include <deque>
#include <unordered_set>
#include <cstdint>

class FileManagerFix42 {
public:
    enum class Mode { Notice, Browse, Actions, Devices, Details, Planning, Confirm, Overwrite, Copy, Summary, ExitConfirm };
    struct Entry { std::string name; bool directory=false,regular=false; std::uint64_t size=0; bool marked=false,copied=false; };
    struct Device { std::string path,label; bool usb=false; };
    struct Panel { std::string path; std::vector<Entry> entries; int selected=0; };
    struct View {
        bool active=false,cancelled=false; Mode mode=Mode::Notice;
        std::array<Panel,2> panels; std::vector<Device> devices;
        int active_panel=0,selection_panel=-1,source_panel=0,action_selected=0,device_selected=0,overwrite_selected=0,exit_selected=0,details_page=0;
        std::string status,selection_path,source,destination,source_directory,destination_directory;
        std::uint64_t done=0,total=0,batch_done=0,batch_total=0,bytes_copied=0,revision=0;
        unsigned file_index=0,file_count=0,folder_count=0,directories_done=0,copied_count=0,skipped_count=0,error_count=0,marked_count=0,selected_items=0;
    };
    explicit FileManagerFix42(std::string root="/dev_hdd0");
    explicit FileManagerFix42(std::vector<std::string> roots);
    ~FileManagerFix42();
    void open(); void close(); void update(const InputFrame&,float dt); void step();
    bool active() const { return view_.active; }
    bool copying() const { return batch_active_; }
    const View& view() const { return view_; }
    const std::vector<Device>& devices() const { return devices_; }
private:
    struct Snapshot {
        bool exists=false,regular=false,directory=false;
        std::uint64_t size=0; std::int64_t mtime=0,ctime=0;
        bool operator==(const Snapshot&) const;
    };
    struct Job { std::string source,destination; Snapshot original; std::size_t root=0; };
    struct Task { std::string source,destination; std::size_t root=0; };
    View view_; std::vector<std::string> roots_; bool discover_usb_=false;
    std::vector<Device> devices_; std::unordered_set<std::string> marked_,copied_;
    std::vector<Job> jobs_; std::deque<Task> pending_; std::vector<std::string> root_items_;
    std::vector<bool> root_ok_; std::size_t job_=0,plan_bytes_=0;
    std::vector<unsigned char> buffer_;
    Task scanning_; Snapshot scanning_snapshot_; int scan_fd_=-1; void* scan_dir_=nullptr; bool scanning_active_=false;
    int source_fd_=-1,dest_fd_=-1,overwrite_all_=0; bool owns_temp_=false,batch_active_=false;
    unsigned serial_=0; std::string temporary_,backup_;
    Snapshot dest_snapshot_,source_fd_snapshot_; float repeat_=0; int repeat_direction_=0;
    Mode exit_previous_=Mode::Browse;
    static Snapshot snapshot(const std::string&,int fd=-1,int* error=nullptr);
    void changed() { ++view_.revision; }
    void discover(); const Device* device_for(const std::string&) const;
    bool entries(const std::string&,std::vector<Entry>&,int& error);
    void refresh(int panel,bool keep=false); void flags(); void clear_selection();
    bool bind_selection(); void mark_one(); void mark_all(); void navigate(const std::string&);
    void prepare(); void plan_step(); bool queue(Task); void planning_finished();
    void start_copy(); void next(); void begin_copy(); void finish(); void skip();
    void release_copy(); void release_scan(); void cancel(const std::string&); void failure(const char*,int error);
    void batch_finished(); std::string unique(const char*);
};
#endif
