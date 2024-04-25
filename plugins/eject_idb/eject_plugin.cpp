// Eject IDB by Elias Bachaalany(c) AllThingsIDA

#include <thread>
#include <functional>
#include <string>
#include <memory>

#ifdef __NT__
    #include <windows.h>
#endif

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

#include "utils.hpp"

class semaphore_helper_t 
{
private:
    std::thread thread_;
    std::string sem_name_;
    qsemaphore_t sem_;
    std::function<void()> callback_;
    bool should_exit_;

public:
    semaphore_helper_t(const char* sem_name, std::function<void()> callback)
        : sem_name_(sem_name), callback_(callback), sem_(nullptr), should_exit_(false) { }

    ~semaphore_helper_t() {
        stop();
    }

    void start() 
    {
        sem_ = qsem_create(sem_name_.c_str(), 0);
        if (sem_ == nullptr)
            return;

        thread_ = std::thread([this]() 
        {
            while (qsem_wait(sem_, -1)) 
            {
                if (should_exit_)
                    break;
                callback_();
            }
        });
    }

    void signal() 
    {
        if (sem_ != nullptr) 
            qsem_post(sem_);
    }

    void stop() 
    {
        if (sem_ != nullptr) 
        {
            should_exit_ = true;
            qsem_post(sem_);
            thread_.join();
            qsem_free(sem_);
            sem_ = nullptr;
        }
    }
};

//--------------------------------------------------------------------------
struct plugin_ctx_t : public plugmod_t
{
    std::unique_ptr<semaphore_helper_t> sem_helper;

    bool disable_ui = false;
    static ssize_t idaapi ui_callback(void* ud, int notification_code, va_list va)
    {
        plugin_ctx_t* ctx = (plugin_ctx_t*)ud;
        return ctx->disable_ui ? 1 : 0;
    }

    void do_eject()
    {
        qstring p = get_idb_path();
        auto idx = p.rfind('.');
        if (idx == qstring::npos)
            return;

        // unfortunately, `save_database` calls the main thread/UI to display success/failure messages
        // thus, before 'ejecting', let's all UI messages
        disable_ui = true;
        flush_buffers();
        qstring new_name = p.substr(0, idx) + ".ejected" + p.substr(idx);
        save_database(new_name.c_str(), DBFL_BAK);
        // Now it is safe to kill IDA.
#ifdef __NT__
        if (MessageBoxW(NULL, L"IDB has been ejected. Do you want to forcefully exit IDA?", L"eject_idb", MB_YESNO | MB_ICONINFORMATION) == IDYES)
            ExitProcess(0);
#endif
        disable_ui = false;
    }

    plugin_ctx_t() 
    {
        char sem_name[64];
        make_semaphore_name(get_idb_path().c_str(), sem_name, sizeof(sem_name));
        sem_helper.reset(
            new semaphore_helper_t(sem_name, std::bind(&plugin_ctx_t::do_eject, this)));
        sem_helper->start();

        msg("eject_idb installed. call the 'eject_idb \"%s\"' command line tool to eject this database!\n",
            get_idb_path().c_str());
        hook_to_notification_point(HT_UI, ui_callback, this);
    }

    // This hanging mode (only if poll is true) can be 'ejected'
    static bool hang_with_loop(bool poll=false)
    {
        while (true)
        {
            if (poll)
                user_cancelled();
            qsleep(1000);
        }
        return true;
    }

    bool idaapi run(size_t arg) override
    {
        switch (arg)
        {
            case 0:
                if (ask_yn(1, "HIDECANCEL\nDo you want to simulate a non-responsive UI hang?") == ASKBTN_YES)
                    hang_with_loop(false);
                break;
            case 1:
                if (ask_yn(1, "HIDECANCEL\nDo you want to simulate a responsive UI hang?") == ASKBTN_YES)
                    hang_with_loop(true);
                break;
        }
        return true;
    }

    ~plugin_ctx_t() override
    {
        //;! use on_event/modern
        unhook_from_notification_point(HT_UI, ui_callback, this);
    }
};

//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    PLUGIN_MULTI,
    []()->plugmod_t* { return new plugin_ctx_t; }, // initialize
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "eject_idb: simulate an infinite loop",
    nullptr
};
