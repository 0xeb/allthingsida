// Eject IDB by Elias Bachaalany(c) AllThingsIDA

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

// Edit the EJECTIDB_HOTKEY_xxx constants to change the hotkey and modifiers
#include "windowthreadhotkey.h"

//--------------------------------------------------------------------------
struct plugin_ctx_t : public plugmod_t
{
    CWindowThreadHotkey w;
    bool disable_ui = false;
    static ssize_t idaapi ui_callback(void* ud, int notification_code, va_list va)
    {
        plugin_ctx_t* ctx = (plugin_ctx_t*)ud;
        return ctx->disable_ui ? 1 : 0;
    }

    plugin_ctx_t() 
    {
        w.Start();
        msg("eject_idb installed. press Ctrl-Alt-E to eject the IDB when IDA UI enters an infinite loop!\n");
        hook_to_notification_point(HT_UI, ui_callback, this);
    }

    // This hanging mode (or an equivalent 'for (;;) {}') cannot be 'ejected'
    static void hang_with_event()
    {
        // Create an unsignaled, manual-reset event
        HANDLE hEvent = CreateEvent(
            NULL,   // default security attributes
            TRUE,   // manual-reset event
            FALSE,  // initial state is nonsignaled
            NULL    // object name (optional)
        );

        if (hEvent == NULL)
        {
            // Handle error
            msg("CreateEvent failed with error: %d\n", GetLastError());
            return;
        }

        // Wait for the event to be signaled
        WaitForSingleObject(hEvent, INFINITE);
        // Close the event handle
        CloseHandle(hEvent);
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
                if (ask_yn(1, "HIDECANCEL\nDo you want to simulate a responsive UI hang?") == ASKBTN_YES)
                    hang_with_loop(true);
                break;
            case 1:
                if (ask_yn(1, "HIDECANCEL\nDo you want to simulate a non-responsive UI hang?") == ASKBTN_YES)
                    hang_with_loop(false);
                break;
            case 2:
                if (ask_yn(1, "HIDECANCEL\nDo you want to simulate a non-responsive UI hang with WaitForSingleObject?") == ASKBTN_YES)
                    hang_with_event();
                break;
        }
        return true;
    }

    ~plugin_ctx_t() override
    {
        unhook_from_notification_point(HT_UI, ui_callback, this);
		w.Stop();
	}
};

plugin_ctx_t* plugin;

//--------------------------------------------------------------------------
extern "C" void do_eject()
{
    qstring p = get_path(PATH_TYPE_IDB);
    auto idx = p.rfind('.');
    if (idx == qstring::npos)
        return;

    // unfortunately, `save_database` calls the main thread/UI to display success/failure messages
    // thus, before 'ejecting', let's all UI messages
    plugin->disable_ui = true;
    flush_buffers();
    qstring new_name = p.substr(0, idx) + ".ejected" + p.substr(idx);
    save_database(new_name.c_str(), DBFL_BAK);
    // Now it is safe to kill IDA.
    if (MessageBoxW(NULL, L"IDB has been ejected. Do you want to forcefully exit IDA?", L"eject_idb", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		ExitProcess(0);
    plugin->disable_ui = false;
}

//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    PLUGIN_FIX | PLUGIN_MULTI,
    []()->plugmod_t* { return plugin = new plugin_ctx_t; }, // initialize
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "eject_idb: simulate UI infinite loop",
    nullptr
};
