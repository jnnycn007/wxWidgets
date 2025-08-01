/////////////////////////////////////////////////////////////////////////////
// Name:        app.h
// Purpose:     interface of wxApp
// Author:      wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


/**
    @class wxAppConsole

    This class is essential for writing console-only or hybrid apps without
    having to define @c wxUSE_GUI=0.

    It is used to:
    @li set and get application-wide properties (see wxAppConsole::CreateTraits
        and wxAppConsole::SetXXX functions)
    @li implement the windowing system message or event loop: events in fact are
        supported even in console-mode applications (see wxAppConsole::HandleEvent
        and wxAppConsole::ProcessPendingEvents);
    @li initiate application processing via wxApp::OnInit;
    @li allow default processing of events not handled by other
        objects in the application (see wxAppConsole::FilterEvent)
    @li implement Apple-specific event handlers (see wxAppConsole::MacXXX functions)

    You should use the macro wxIMPLEMENT_APP(appClass) in your application
    implementation file to tell wxWidgets how to create an instance of your
    application class.

    Use wxDECLARE_APP(appClass) in a header file if you want the ::wxGetApp() function
    (which returns a reference to your application object) to be visible to other
    files.

    @library{wxbase}
    @category{appmanagement}

    @see @ref overview_app, wxApp, wxAppTraits, wxEventLoopBase
*/
class wxAppConsole : public wxEvtHandler,
                     public wxEventFilter
{
protected:
    /**
        Creates the wxAppTraits object when GetTraits() needs it for the first time.

        @see wxAppTraits
    */
    virtual wxAppTraits* CreateTraits();

public:

    /**
        Destructor.
    */
    virtual ~wxAppConsole();


    /**
        @name Event-handling

        Note that you should look at wxEvtLoopBase for more event-processing
        documentation.
    */
    ///@{

    /**
        Called by wxWidgets on creation of the application. Override this if you wish
        to provide your own (environment-dependent) main loop.

        @return 0 under X, and the wParam of the WM_QUIT message under Windows.
    */
    virtual int MainLoop();

    /**
        Call this to explicitly exit the main message (event) loop.
        You should normally exit the main loop (and the application) by deleting
        the top window.

        This function simply calls wxEvtLoopBase::Exit() on the active loop.
    */
    virtual void ExitMainLoop();

    /**
        Overridden wxEventFilter method.

        This function is called before processing any event and allows the application
        to preempt the processing of some events, see wxEventFilter
        documentation for more information.

        wxApp implementation of this method always return -1 indicating that
        the event should be processed normally.
    */
    virtual int FilterEvent(wxEvent& event);

    /**
        Returns the main event loop instance, i.e.\ the event loop which is started
        by OnRun() and which dispatches all events sent from the native toolkit
        to the application (except when new event loops are temporarily set-up).
        The returned value maybe @NULL. Put initialization code which needs a
        non-null main event loop into OnEventLoopEnter().
    */
    wxEventLoopBase* GetMainLoop() const;

    /**
        This function simply invokes the given method @a func of the specified
        event handler @a handler with the @a event as parameter. It exists solely
        to allow to catch the C++ exceptions which could be thrown by all event
        handlers in the application in one place: if you want to do this, override
        this function in your wxApp-derived class and add try/catch clause(s) to it.
    */
    virtual void HandleEvent(wxEvtHandler* handler,
                             wxEventFunction func,
                             wxEvent& event) const;

    /**
        Returns @true if the application is using an event loop.

        This function always returns @true for the GUI applications which
        must use an event loop but by default only returns @true for the
        console programs if an event loop is already running as it can't know
        whether one will be created in the future.

        Thus, it only makes sense to override it in console applications which
        do use an event loop, to return @true instead of checking if there is a
        currently active event loop.
     */
    virtual bool UsesEventLoop() const;

    ///@}


    /**
        @name Pending events

        Pending events are handled by wxAppConsole rather than wxEventLoopBase
        to allow queuing of events even when there's no event loop
        (e.g. in wxAppConsole::OnInit).
    */
    ///@{

    /**
        Process all pending events; it is necessary to call this function to
        process events posted with wxEvtHandler::QueueEvent or wxEvtHandler::AddPendingEvent.

        This happens during each event loop iteration (see wxEventLoopBase) in GUI mode but
        it may be also called directly.

        Note that this function does not only process the pending events for the wxApp object
        itself (which derives from wxEvtHandler) but also the pending events for @e any
        event handler of this application.

        This function will immediately return and do nothing if SuspendProcessingOfPendingEvents()
        was called.
    */
    virtual void ProcessPendingEvents();

    /**
        Deletes the pending events of all wxEvtHandlers of this application.

        See wxEvtHandler::DeletePendingEvents() for warnings about deleting the pending
        events.
    */
    void DeletePendingEvents();

    /**
        Returns @true if there are pending events on the internal pending event list.

        Whenever wxEvtHandler::QueueEvent or wxEvtHandler::AddPendingEvent() are
        called (not only for wxApp itself, but for any event handler of the application!),
        the internal wxApp's list of handlers with pending events is updated and this
        function will return true.
    */
    bool HasPendingEvents() const;

    /**
        Temporary suspends processing of the pending events.

        @see ResumeProcessingOfPendingEvents()
    */
    void SuspendProcessingOfPendingEvents();

    /**
        Resume processing of the pending events previously stopped because of a
        call to SuspendProcessingOfPendingEvents().
    */
    void ResumeProcessingOfPendingEvents();

    ///@}

    /**
        Delayed objects destruction.

        In applications using events it may be unsafe for an event handler to
        delete the object which generated the event because more events may be
        still pending for the same object. In this case the handler may call
        ScheduleForDestruction() instead.
     */
    ///@{

    /**
        Schedule the object for destruction in the near future.

        Notice that if the application is not using an event loop, i.e. if
        UsesEventLoop() returns @false, this method will simply delete the
        object immediately.

        Examples of using this function inside wxWidgets itself include
        deleting the top level windows when they are closed and sockets when
        they are disconnected.
     */
    void ScheduleForDestruction(wxObject *object);

    /**
        Check if the object had been scheduled for destruction with
        ScheduleForDestruction().

        This function may be useful as an optimization to avoid doing something
        with an object which will be soon destroyed in any case.
     */
    bool IsScheduledForDestruction(wxObject *object) const;

    ///@}


    /**
        Yields control to pending messages in the event loop.

        This method is a convenient wrapper for wxEvtLoopBase::Yield(). If the
        main loop is currently running, it calls this method on it. Otherwise
        it creates a temporary event loop and uses it instead, which can be
        useful to process pending messages during the program startup, before
        the main loop is created.

        Use extreme caution when calling this function as, just as
        wxEvtLoopBase::Yield(), it can result in unexpected reentrances.
     */
    bool Yield(bool onlyIfNeeded = false);

    /**
        Allows external code to modify global ::wxTheApp, but you should really
        know what you're doing if you call it.

        @param app
            Replacement for the global application object.

        @see GetInstance()
    */
    static void SetInstance(wxAppConsole* app);

    /**
        Returns the one and only global application object.
        Usually ::wxTheApp is used instead.

        @see SetInstance(), wxApp::GetGUIInstance()
    */
    static wxAppConsole* GetInstance();

    /**
        Returns @true if the main event loop is currently running, i.e.\ if the
        application is inside OnRun().

        This can be useful to test whether events can be dispatched. For example,
        if this function returns @false, non-blocking sockets cannot be used because
        the events from them would never be processed.
    */
    static bool IsMainLoopRunning();

    /**
        @name Callbacks for application-wide "events"
    */
    ///@{

    /**
        This function is called when an assert failure occurs, i.e.\ the condition
        specified in wxASSERT() macro evaluated to @false.

        It is only called in debug mode (when @c \__WXDEBUG__ is defined) as
        asserts are not left in the release code at all.
        The base class version shows the default assert failure dialog box proposing to
        the user to stop the program, continue or ignore all subsequent asserts.

        @param file
            the name of the source file where the assert occurred
        @param line
            the line number in this file where the assert occurred
        @param func
            the name of the function where the assert occurred
        @param cond
            the condition of the failed assert in text form
        @param msg
            the message specified as argument to wxASSERT_MSG or wxFAIL_MSG, will
            be @NULL if just wxASSERT or wxFAIL was used
    */
    virtual void OnAssertFailure(const wxChar *file,
                                 int line,
                                 const wxChar *func,
                                 const wxChar *cond,
                                 const wxChar *msg);

    /**
        Called when command line parsing fails (i.e.\ an incorrect command line option
        was specified by the user). The default behaviour is to show the program usage
        text and abort the program.

        Return @true to continue normal execution or @false to return
        @false from OnInit() thus terminating the program.

        @see OnInitCmdLine()
    */
    virtual bool OnCmdLineError(wxCmdLineParser& parser);

    /**
        Called when the help option (@c \--help) was specified on the command line.
        The default behaviour is to show the program usage text and abort the program.

        Return @true to continue normal execution or @false to return
        @false from OnInit() thus terminating the program.

        @see OnInitCmdLine()
    */
    virtual bool OnCmdLineHelp(wxCmdLineParser& parser);

    /**
        Called after the command line had been successfully parsed. You may override
        this method to test for the values of the various parameters which could be
        set from the command line.

        Don't forget to call the base class version unless you want to suppress
        processing of the standard command line options.
        Return @true to continue normal execution or @false to return @false from
        OnInit() thus terminating the program.

        @see OnInitCmdLine()
    */
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

    /**
        Called by wxEventLoopBase::SetActive(): you can override this function
        and put here the code which needs an active event loop.

        Note that this function is called whenever an event loop is activated;
        you may want to use wxEventLoopBase::IsMain() to perform initialization
        specific for the app's main event loop.

        @see OnEventLoopExit()
    */
    virtual void OnEventLoopEnter(wxEventLoopBase* loop);

    /**
        Called by wxEventLoopBase::OnExit() for each event loop which
        is exited.

        @see OnEventLoopEnter()
    */
    virtual void OnEventLoopExit(wxEventLoopBase* loop);

    /**
        Override this member function for any processing which needs to be
        done as the application is about to exit. OnExit is called after
        destroying all application windows and controls, but before
        wxWidgets cleanup. Note that it is not called at all if
        OnInit() failed.

        The return value of this function is currently ignored, return the same
        value as returned by the base class method if you override it.
    */
    virtual int OnExit();

    /**
        This function may be called if something fatal happens: an unhandled
        exception under Win32 or a fatal signal under Unix, for example. However,
        this will not happen by default: you have to explicitly call
        wxHandleFatalExceptions() to enable this.

        Generally speaking, this function should only show a message to the user and
        return. You may attempt to save unsaved data but this is not guaranteed to
        work and, in fact, probably won't.

        @see wxHandleFatalExceptions()
    */
    virtual void OnFatalException();

    /**
        This must be provided by the application, and will usually create the
        application's main window, optionally calling SetTopWindow().

        You may use OnExit() to clean up anything initialized here, provided
        that the function returns @true.

        Notice that if you want to use the command line processing provided by
        wxWidgets (see OnInitCmdLine() and OnCmdLineParsed() functions) you
        have to call the base class version in the derived class OnInit(),
        e.g.:

        @code
        bool MyApp::OnInit() {
            if ( !wxApp::OnInit() ) {
                // The most likely reason for the error here is that incorrect
                // command line arguments have been specified, so just exit:
                // error message has already been given.
                return false;
            }

            // Perform any additional initialization here.

            return true;
        }
        @endcode

        Return @true to continue processing, @false to exit the application
        immediately. In the latter case, you may want to call SetErrorExitCode()
        to set the process exit code to use when the application terminates.
    */
    virtual bool OnInit();

    /**
        Called from OnInit() and may be used to initialize the parser with the
        command line options for this application. The base class versions adds
        support for a few standard options only.

        Note that this method should just configure @a parser to accept the
        desired command line options by calling wxCmdLineParser::AddOption(),
        wxCmdLineParser::AddSwitch() and similar methods, but should @e not
        call wxCmdLineParser::Parse() as this will be done by wxWidgets itself
        slightly later.
    */
    virtual void OnInitCmdLine(wxCmdLineParser& parser);

    /**
        Virtual function executing the application's main event loop.

        For the GUI applications, it is typically not necessary to override
        this function, as the default implementation, which enters the main
        event loop and dispatches all events until ExitMainLoop() is called
        (either explicitly or because the last top level window was closed),
        rarely needs to be customized.

        For the console applications not using event loops, this function can
        be used as the equivalent of the traditional @c main() function by
        putting most of the program logic here.

        The return value of this function becomes the exit code of the program,
        so it should return 0 in case of successful termination.

        Note that this function is not called at all if OnInit() had returned
        @false.
    */
    virtual int OnRun();

    ///@}


    /**
        @name Exceptions support

        Methods related to C++ exceptions handling.

        @see overview_exceptions
    */
    ///@{

    /**
        This function is called if an unhandled exception occurs inside the main
        application event loop. It can return @true to ignore the exception and to
        continue running the loop or @false to exit the loop and terminate the
        program.

        The default behaviour of this function is the latter in all ports except under
        Windows where a dialog is shown to the user which allows him to choose between
        the different options. You may override this function in your class to do
        something more appropriate.

        If this method rethrows the exception and if the exception can't be
        stored for later processing using StoreCurrentException(), the program
        will terminate after calling OnUnhandledException().

        You should consider overriding this method to perform whichever last
        resort exception handling that would be done in a typical C++ program
        in a @c try/catch block around the entire @c main() function. As this
        method is called during exception handling, you may use the C++ @c
        throw keyword to rethrow the current exception to catch it again and
        analyze it. For example:

        @code
        class MyApp : public wxApp {
        public:
            virtual bool OnExceptionInMainLoop()
            {
                wxString error;
                try {
                    throw; // Rethrow the current exception.
                } catch (const MyException& e) {
                    error = e.GetMyErrorMessage();
                } catch (const std::exception& e) {
                    error = e.what();
                } catch ( ... ) {
                    error = "unknown error.";
                }

                wxLogError("Unexpected exception has occurred: %s, the program will terminate.", error);

                // Exit the main loop and thus terminate the program.
                return false;
            }
        };
        @endcode
    */
    virtual bool OnExceptionInMainLoop();

    /**
        This function is called when an unhandled C++ exception occurs in user
        code called by wxWidgets.

        Any unhandled exceptions thrown from (overridden versions of) OnInit()
        and OnExit() methods as well as any exceptions thrown from inside the
        main loop and re-thrown by OnUnhandledException() will result in a call
        to this function.

        By the time this function is called, the program is already about to
        exit and the exception can't be handled nor ignored any more, override
        OnUnhandledException() or use explicit @c try/catch blocks around
        OnInit() body to be able to handle the exception earlier.

        The default implementation dumps information about the exception using
        wxMessageOutputBest.

        @note This function should _not_ throw any exceptions itself.
    */
    virtual void OnUnhandledException();

    /**
        Call OnUnhandledException() on the current wxTheApp object if it exists.

        This function is used by wxWidgets itself and is usually not meant to
        be called by the application code. If you do call it, it must be done
        from a `catch` clause of a `try` block, i.e. there must be a currently
        handled exception.

        The function checks if ::wxTheApp is not @NULL and if it is, calls
        OnUnhandledException() on it.

        Additionally, if this call results in an exception, it is caught and
        wxAppConsole::OnUnhandledException() is called.

        @since 3.3.0
     */
    static void CallOnUnhandledException();

    /**
        Method to store exceptions not handled by OnExceptionInMainLoop().

        @note The default implementation of this function when using C++98
            compiler just returns false, as there is no generic way to store an
            arbitrary exception in C++98 and each application must do it on its
            own for the exceptions it uses in its overridden version. When
            using C++11, the default implementation uses
            std::current_exception() and returns true, so it's normally not
            necessary to override this method when using C++11.

        This function can be overridden to store the current exception, in view
        of rethrowing it later when RethrowStoredException() is called. If the
        exception was stored, return true. If the exception can't be stored,
        i.e. if this function returns false, the program will abort after
        calling OnUnhandledException().

        It is necessary to override this function if OnExceptionInMainLoop()
        doesn't catch all exceptions, but you still want to handle them using
        explicit @c try/catch statements. Typical use could be to allow code
        like the following to work:

        @code
        void MyFrame::SomeFunction()
        {
            try {
                MyDialog dlg(this);
                dlg.ShowModal();
            } catch ( const MyExpectedException& e ) {
                // Deal with the exceptions thrown from the dialog.
            }
        }
        @endcode

        By default, throwing an exception from an event handler called from the
        dialog modal event loop would terminate the application as the
        exception can't be safely propagated to the code in the catch clause
        because of the presence of the native system functions (through which
        C++ exceptions can't, generally speaking, propagate) in the call stack
        between them.

        Overriding this method allows the exception to be stored when it is
        detected and rethrown using RethrowStoredException() when the native
        system function dispatching the dialog events terminates, with the
        result that the code above works as expected.

        An example of implementing this method:
        @code
        class MyApp : public wxApp {
        public:
            virtual bool StoreCurrentException()
            {
                try {
                    throw;
                } catch ( const std::runtime_exception& e ) {
                    if ( !m_runtimeError.empty() ) {
                        // This is not supposed to happen, only one exception,
                        // at most, should be stored.
                        return false;
                    }

                    m_runtimeError = e.what();

                    // Don't terminate, let our code handle this exception later.
                    return true;
                } catch ( ... ) {
                    // This could be extended to store information about any
                    // other exceptions too, but if we don't store them, we
                    // should return false to let the program die.
                }

                return false;
            }

            virtual void RethrowStoredException()
            {
                if ( !m_runtimeError.empty() ) {
                    std::runtime_exception e(m_runtimeError);
                    m_runtimeError.clear();
                    throw e;
                }
            }

        private:
            std::string m_runtimeError;
        };
        @endcode

        @see OnExceptionInMainLoop(), RethrowStoredException()

        @since 3.1.0
    */
    virtual bool StoreCurrentException();

    /**
        Method to rethrow exceptions stored by StoreCurrentException().

        @note Just as with StoreCurrentException(), it is usually not necessary
            to override this method when using C++11.

        If StoreCurrentException() is overridden, this function should be
        overridden as well to rethrow the exceptions stored by it when the
        control gets back to our code, i.e. when it's safe to do it.

        See StoreCurrentException() for an example of implementing this method.

        The default version does nothing when using C++98 and uses
        std::rethrow_exception() in C++11.

        @since 3.1.0
    */
    virtual void RethrowStoredException();

    ///@}


    /**
        @name Application information
    */
    ///@{

    /**
        Returns the user-readable application name.

        The difference between this string and the one returned by GetAppName()
        is that this one is meant to be shown to the user and so should be used
        for the window titles, page headers and so on while the other one
        should be only used internally, e.g. for the file names or
        configuration file keys.

        If the application name for display had been previously set by
        SetAppDisplayName(), it will be returned by this function. Otherwise,
        if SetAppName() had been called its value will be returned; also as is.
        Finally if none was called, this function returns the program name
        capitalized using wxString::Capitalize().

        @since 2.9.0
    */
    wxString GetAppDisplayName() const;

    /**
        Returns the application name.

        If SetAppName() had been called, returns the string passed to it.
        Otherwise returns the program name, i.e. the value of @c argv[0] passed
        to the @c main() function.

        @see GetAppDisplayName()
    */
    wxString GetAppName() const;

    /**
        Gets the class name of the application. The class name may be used in a
        platform specific manner to refer to the application.

        @see SetClassName()
    */
    wxString GetClassName() const;

    /**
        Returns a pointer to the wxAppTraits object for the application.
        If you want to customize the wxAppTraits object, you must override the
        CreateTraits() function.
    */
    wxAppTraits* GetTraits();

    /**
        Returns the user-readable vendor name. The difference between this string
        and the one returned by GetVendorName() is that this one is meant to be shown
        to the user and so should be used for the window titles, page headers and so on
        while the other one should be only used internally, e.g. for the file names or
        configuration file keys.

        By default, returns the same string as GetVendorName().

        @since 2.9.0
    */
    const wxString& GetVendorDisplayName() const;

    /**
        Returns the application's vendor name.
    */
    const wxString& GetVendorName() const;

    /**
        Set the application name to be used in the user-visible places such as
        window titles.

        See GetAppDisplayName() for more about the differences between the
        display name and name.

        Notice that if this function is called, the name is used as is, without
        any capitalization as done by default by GetAppDisplayName().
    */
    void SetAppDisplayName(const wxString& name);

    /**
        Sets the name of the application. This name should be used for file names,
        configuration file entries and other internal strings. For the user-visible
        strings, such as the window titles, the application display name set by
        SetAppDisplayName() is used instead.

        By default the application name is set to the name of its executable file.

        @see GetAppName()
    */
    void SetAppName(const wxString& name);

    /**
        Sets the class name of the application.

        The class name is used in a platform specific manner. Currently it is
        used as "Application User Model ID" under Windows (see [Microsoft
        documentation][microsoft-docs]), "app ID" when using wxGTK 3.24.22 or
        later with Wayland (see [Wayland documentation][wayland-docs]) and is
        unused under the other platforms.

        [microsoft-docs]: https://learn.microsoft.com/en-us/windows/win32/shell/appids
        [wayland-docs]: https://wayland.app/protocols/xdg-shell#xdg_toplevel:request:set_app_id

        When it is used, the class name purpose is to allow the system to
        handle all windows with the same ID as belonging to the same
        application, e.g. to group them together in the taskbar (so the value
        set here is used by wxTaskBarJumpList constructor). By default the
        application executable name is used as its ID, so it is not necessary
        to set the class name, but it may be useful to do it to specify a more
        unique string (typically by using a reverse domain name notation with
        the domain unique to the application vendor) or by specifying the same
        ID in different applications that should be handled as a single one at
        UI level.

        @note Under Windows setting the application user model ID changes some
        functionality available by default, notably Shift middle clicking the
        application icon in the taskbar doesn't open a new instance of the
        application any more and most recently used files list maintained by
        the shell doesn't work any longer. Application that need to keep this
        working need to use `SHGetPropertyStoreForWindow()` and
        `SHAddToRecentDocs()` functions to provide the necessary support for it.

        Please note that SetClassName() must be called as early as possible and
        definitely before creating any top-level windows to have an effect.
        Typically it should be called in the constructor of the class derived
        from wxApp, e.g.

        @code
        class MyApp : public wxApp
        {
        public:
            MyApp() {
                // Constructor shouldn't perform any non-trivial initialization
                // as the GUI is not available yet, but this function is fine
                // to call.
                SetClassName("com.example.myapp");
            }
        };
        @endcode

        @see GetClassName()
    */
    void SetClassName(const wxString& name);

    /**
        Set the vendor name to be used in the user-visible places.
        See GetVendorDisplayName() for more about the differences between the
        display name and name.
    */
    void SetVendorDisplayName(const wxString& name);

    /**
        Sets the name of application's vendor. The name will be used
        in registry access. A default name is set by wxWidgets.

        @see GetVendorName()
    */
    void SetVendorName(const wxString& name);

    ///@}

    /**
        Sets the C locale to the default locale for the current environment.

        It is advised to call this to ensure that the underlying toolkit uses
        the locale in which the numbers and monetary amounts are shown in the
        format expected by user and so on.

        Calling this function is roughly equivalent to calling
        @code
            setlocale(LC_ALL, "");
        @endcode
        but performs additional toolkit-specific tasks under some platforms and
        so should be used instead of @c setlocale() itself. Alternatively, you
        can use wxLocale to change the locale with more control.

        Notice that this does @em not change the global C++ locale, you need to
        do it explicitly if you want, e.g.
        @code
            std::locale::global(std::locale(""));
        @endcode
        but be warned that locale support in C++ standard library can be poor
        or worse under some platforms.

        @since 2.9.5
     */
    void SetCLocale();

    /**
        Sets the error code to use in case of exit on error.

        This function is mostly useful to customize the error code returned by
        the application when it exits due to OnInit() returning @false and can
        be called from OnInit() itself or other virtual functions called from
        it, for example OnCmdLineError().

        By default, the exit code is @c 255 which indicates a generic error,
        so it is may be useful to call this function to set a more precise exit
        code, e.g. @c 2 which is a de facto standard exit code if command line
        parsing fails.

        Please also note that in the previous versions of wxWidgets this exit
        code was @c -1, which corresponds to either @c 255 or @c 127 depending
        on the platform and compiler used, so you may want to call this
        function with @c -1 argument if you need to preserve compatibility with
        the old behaviour.

        SetErrorExitCode() can be overridden by the application to perform
        additional actions, but the overridden version should call the base
        class version to update the value returned by GetErrorExitCode() and
        actually used when exiting the application.

        @see SetFatalErrorExitCode()

        @since 3.3.0
     */
    virtual void SetErrorExitCode(int code);

    /**
        Allows to set a custom process exit code if a fatal error happens.

        If the program can't continue due to a fatal error, such as receiving
        an unhandled exception or failing to initialize the graphical
        environment for the GUI applications, it terminates with the default
        fatal error exit code which is @c 255.

        This function can be used to change this default value to something
        else, e.g. @c -1 which used to be returned in the previous versions of
        wxWidgets (and corresponds to either @c 255 or @c 127 depending on the
        platform and compiler used) if compatibility is important.

        Notice that it has to be called as early as possible to take effect
        even during the early application initialization, e.g.

        @code
        struct FatalErrorCodeInitializer {
            FatalErrorCodeInitializer() {
                wxApp::SetFatalErrorExitCode(3); // same as abort()
            }
        };

        // Create a global variable to call SetFatalErrorExitCode() in its ctor.
        static FatalErrorCodeInitializer s_fatalErrorCodeInitializer;
        @endcode

        Note that this function doesn't change the exit code returned if
        OnInit() returns @false, so if you change the default value of this
        exit code you may want to call SetErrorExitCode() to change the other
        one too.

        @since 3.3.0
     */
    static void SetFatalErrorExitCode(int code);


    /**
        Number of command line arguments (after environment-specific processing).
    */
    int argc;

    /**
        Command line arguments (after environment-specific processing).

        Under Windows and Linux/Unix, you should parse the command line
        arguments and check for files to be opened when starting your
        application. Under macOS, you need to override MacOpenFiles()
        since command line arguments are used differently there.

        You may use the wxCmdLineParser to parse command line arguments.
    */
    wxChar** argv;
};




/**
    @class wxApp

    The wxApp class represents the application itself when @c wxUSE_GUI=1.

    In addition to the features provided by wxAppConsole it keeps track of
    the <em>top window</em> (see SetTopWindow()) and adds support for
    video modes (see SetDisplayMode()).

    In general, application-wide settings for GUI-only apps are accessible
    from wxApp (or from wxSystemSettings or wxSystemOptions classes).

    @beginEventEmissionTable
    @event{EVT_QUERY_END_SESSION(func)}
        Process a query end session event, supplying the member function.
        See wxCloseEvent.
    @event{EVT_END_SESSION(func)}
        Process an end session event, supplying the member function.
        See wxCloseEvent.
    @event{EVT_ACTIVATE_APP(func)}
        Process a @c wxEVT_ACTIVATE_APP event. See wxActivateEvent.
    @event{EVT_HIBERNATE(func)}
        Process a hibernate event. See wxActivateEvent.
    @event{EVT_DIALUP_CONNECTED(func)}
        A connection with the network was established. See wxDialUpEvent.
    @event{EVT_DIALUP_DISCONNECTED(func)}
        The connection with the network was lost. See wxDialUpEvent.
    @event{EVT_IDLE(func)}
        Process a @c wxEVT_IDLE event. See wxIdleEvent.
    @endEventTable

    @library{wxbase}
    @category{appmanagement}

    @see @ref overview_app, wxAppTraits, wxEventLoopBase, wxSystemSettings
*/
class wxApp : public wxAppConsole
{
public:
    /**
        Constructor. Called implicitly with a definition of a wxApp object.
    */
    wxApp();

    /**
        Destructor. Will be called implicitly on program exit if the wxApp
        object is created on the stack.
    */
    virtual ~wxApp();

    /**
        Get display mode that is used use. This is only used in framebuffer
        wxWidgets ports such as wxDFB.
    */
    virtual wxVideoMode GetDisplayMode() const;

    /**
        Returns the current GUI wxApp object if any or @NULL otherwise.

        This function should only be used in the rare cases when the same code
        needs to work in both console and GUI applications, but needs to use
        GUI-specific functionality if it is available, and so just calling
        wxAppConsole::GetInstance() is insufficient while using ::wxTheApp is
        incorrect, as the application object is not always a GUI wxApp.

        For example:
        @code
            WXWidget handle = 0;
            if ( wxApp* const app = wxApp::GetGUIInstance() ) {
                if ( wxWindow* const w = app->GetTopWindow() ) {
                    handle = w->GetHandle();
                }
            }
            //else: no window to use

            some_native_function_taking_a_window_handle(handle);
        @endcode

        Note that in this particular example, you could  use GetMainTopWindow()
        which already does the same thing instead of doing it yourself.

        @since 3.1.6
    */
    static wxAppConsole* GetGUIInstance();

    /**
        Returns @true if the application will exit when the top-level frame is deleted.

        @see SetExitOnFrameDelete()
    */
    bool GetExitOnFrameDelete() const;

    /**
        Return the layout direction for the current locale or @c wxLayout_Default
        if it's unknown.
    */
    virtual wxLayoutDirection GetLayoutDirection() const;

    /**
        Returns @true if the application will use the best visual on systems that support
        different visuals, @false otherwise.

        @see SetUseBestVisual()
    */
    bool GetUseBestVisual() const;

    /**
        Returns a pointer to the top application window if any.

        This function is safe to call even before creating, or after
        destroying, the application object, as it simply returns @NULL if it
        doesn't exist. Otherwise it's equivalent to calling
        @c wxTheApp->GetTopWindow().

        @since 3.1.5
     */
    static wxWindow* GetMainTopWindow();

    /**
        Returns a pointer to the top window.

        @remarks
            If the top window hasn't been set using SetTopWindow(), this function
            will find the first top-level window (frame or dialog or instance of
            wxTopLevelWindow) from the internal top level window list and return that.

        @see SetTopWindow()
    */
    virtual wxWindow* GetTopWindow() const;

    /**
        Returns @true if the application is active, i.e.\ if one of its windows is
        currently in the foreground.

        If this function returns @false and you need to attract users attention to
        the application, you may use wxTopLevelWindow::RequestUserAttention to do it.
    */
    virtual bool IsActive() const;

    /**
        This function is similar to wxYield(), except that it disables the user
        input to all program windows before calling wxAppConsole::Yield and re-enables it
        again afterwards. If @a win is not @NULL, this window will remain enabled,
        allowing the implementation of some limited user interaction.
        Returns the result of the call to wxAppConsole::Yield.

        @see wxSafeYield
    */
    virtual bool SafeYield(wxWindow *win, bool onlyIfNeeded);

    /**
        Works like SafeYield() with @e onlyIfNeeded == @true except that
        it allows the caller to specify a mask of events to be processed.

        See wxEventLoopBase::YieldFor() for more info.
    */
    virtual bool SafeYieldFor(wxWindow *win, long eventsToProcess);

    /**
        Windows-only function for processing a message. This function is called
        from the main message loop, checking for windows that may wish to process it.

        The function returns @true if the message was processed, @false otherwise.
        If you use wxWidgets with another class library with its own message loop,
        you should make sure that this function is called to allow wxWidgets to
        receive messages. For example, to allow co-existence with the Microsoft
        Foundation Classes, override the PreTranslateMessage function:

        @code
        // Provide wxWidgets message loop compatibility
        BOOL CTheApp::PreTranslateMessage(MSG *msg)
        {
            if (wxTheApp && wxTheApp->ProcessMessage((WXMSW *)msg))
                return true;
            else
                return CWinApp::PreTranslateMessage(msg);
        }
        @endcode

        @onlyfor{wxmsw}
    */
    bool ProcessMessage(WXMSG* msg);


    /**
        Possible parameters for SetAppearance().

        @since 3.3.0
    */
    enum class Appearance
    {
        System, ///< Use system default appearance.
        Light,  ///< Use light appearance.
        Dark    ///< Use dark appearance.
    };

    /**
        Possible values returned by SetAppearance().

        @since 3.3.0
    */
    enum class AppearanceResult
    {
        Failure,     ///< Changing the appearance failed.
        Ok,          ///< Appearance was successfully changed.
        CannotChange ///< Appearance can't be changed any more.
    };

    /**
        Request using either system default or explicitly light or dark theme
        for the application.

        Under GTK and macOS applications use the system default appearance by
        default, and so it is only useful to call this function with either
        Appearance::Light or Appearance::Dark parameters if you need to
        override the default system appearance. The effect of calling this
        function is immediate, i.e. this function returns
        AppearanceResult::Ok, and affects all the existing windows as well
        as any windows created after this call.

        Under MSW, the default appearance is always light and the applications
        that want to follow the system appearance need to explicitly call this
        function with Appearance::System parameter in order to do it. Please
        note using dark appearance under MSW requires using non-documented
        system functions and has several known limitations, please see
        MSWEnableDarkMode() for more details. Also, on this platform the
        appearance can be only set before any windows are created and calling
        this function too late will return AppearanceResult::CannotChange.

        Note that to query the current appearance, you can use
        wxSystemAppearance, see wxSystemSettings::GetAppearance().

        @return AppearanceResult::Ok if the appearance was successfully
            changed or had been already set to the requested value,
            AppearanceResult::CannotChange if the appearance can't be changed
            any more because it's too late to do it but could be changed if
            done immediately on next program launch (only returned by wxMSW
            currently) or AppearanceResult::Failure if changing the appearance
            failed for some other reason, e.g. because `GTK_THEME` is defined
            when using wxGTK of this function is not implemented at all for
            the current platform.

        @since 3.3.0
    */
    AppearanceResult SetAppearance(Appearance appearance);

    /**
        Set display mode to use. This is only used in framebuffer wxWidgets
        ports such as wxDFB.
    */
    virtual bool SetDisplayMode(const wxVideoMode& info);

    /**
        Allows the programmer to specify whether the application will exit when the
        top-level frame is deleted.

        @param flag
            If @true (the default), the application will exit when the top-level frame
            is deleted. If @false, the application will continue to run.

        @see GetExitOnFrameDelete(), @ref overview_app_shutdown
    */
    void SetExitOnFrameDelete(bool flag);

    /**
        Allows runtime switching of the UI environment theme.

        Currently implemented for wxGTK2-only.
        Return @true if theme was successfully changed.

        @param theme
            The name of the new theme or an absolute path to a gtkrc-theme-file
    */
    virtual bool SetNativeTheme(const wxString& theme);

    /**
        Sets the 'top' window. You can call this from within OnInit() to let wxWidgets
        know which is the main window. You don't have to set the top window;
        it is only a convenience so that (for example) certain dialogs without parents
        can use a specific window as the top window.

        If no top window is specified by the application, wxWidgets just uses the
        first frame or dialog (or better, any wxTopLevelWindow) in its top-level
        window list, when it needs to use the top window.
        If you previously called SetTopWindow() and now you need to restore this
        automatic behaviour you can call `wxApp::SetTopWindow(nullptr)`.

        @param window
            The new top window.

        @see GetTopWindow(), OnInit()
    */
    void SetTopWindow(wxWindow* window);

    /**
        Allows the programmer to specify whether the application will use the best
        visual on systems that support several visual on the same display. This is typically
        the case under Solaris and IRIX, where the default visual is only 8-bit whereas
        certain applications are supposed to run in TrueColour mode.

        Note that this function has to be called in the constructor of the wxApp
        instance and won't have any effect when called later on.
        This function currently only has effect under GTK.

        @param flag
            If @true, the app will use the best visual.
        @param forceTrueColour
            If @true then the application will try to force using a TrueColour
            visual and abort the app if none is found.
    */
    void SetUseBestVisual(bool flag, bool forceTrueColour = false);

    /**
        @name GTK-specific functions
    */
    ///@{

    /**
        Disables the printing of various GTK messages.

        This function can be called to suppress GTK diagnostic messages that
        are output on the standard error stream by default.

        If @c WXSUPPRESS_GTK_DIAGNOSTICS environment variable is set to a
        non-zero value, wxWidgets automatically calls this function on program
        startup with the value of this variable as @a flags if it's a number or
        with the default flags value otherwise.

        The default value of the argument disables all messages, but you
        can pass in a mask flag to specifically disable only particular
        categories of messages.

        Note that this function only works when using glib 2.50 (released in
        September 2016) or later and does nothing with the older versions of
        the library.

        @param flags
           The mask for the types of messages to suppress. Refer to the
           glib documentation for the @c GLogLevelFlags enum, which defines
           the various message types.

        @onlyfor{wxgtk}

        @since 3.1.6
    */
    static void GTKSuppressDiagnostics(int flags = -1);

    /**
        Allows wxWidgets to selectively suppress some GTK messages.

        This function can be called to allow wxWidgets to control GTK message
        logging. You must @e not call it if your application calls the @c
        g_log_set_writer_func() function itself, as this function can be only
        called once.

        It is recommended to call this function in your overridden version of
        wxApp::OnInit() to allow wxWidgets to suppress some spurious GTK error
        messages, e.g. the ones that happen whenever wxNotebook pages are
        removed with the current GTK versions.

        @onlyfor{wxgtk}

        @since 3.2.1
     */
    static void GTKAllowDiagnosticsControl();

    ///@}

    /**
        @name Mac-specific functions
    */
    ///@{

    /**
        Called in response of an "open-application" Apple event.
        Override this to create a new document in your app.

        @onlyfor{wxosx}
    */
    virtual void MacNewFile();

    /**
        Called in response of an openFiles message.

        You need to override this method in order to open one or more document
        files after the user double clicked on it or if the files and/or
        folders were dropped on either the application in the dock or the
        application icon in Finder.

        By default this method calls MacOpenFile for each file/folder.

        @onlyfor{wxosx}

        @since 2.9.3
    */
    virtual void MacOpenFiles(const wxArrayString& fileNames);

    /**
        Called in response of an "open-document" Apple event.

        @deprecated
        This function is kept mostly for backwards compatibility. Please
        override wxApp::MacOpenFiles method instead in any new code.

        @onlyfor{wxosx}
    */
    virtual void MacOpenFile(const wxString& fileName);

    /**
        Called in response of a "get-url" Apple event.

        @onlyfor{wxosx}
    */
    virtual void MacOpenURL(const wxString& url);

    /**
        Called in response of a "print-document" Apple event.

        @onlyfor{wxosx}
    */
    virtual void MacPrintFile(const wxString& fileName);

    /**
        Called in response of a "reopen-application" Apple event.

        @onlyfor{wxosx}
    */
    virtual void MacReopenApp();

    /**
        May be overridden to indicate that the application is not a foreground
        GUI application under macOS.

        This method is called during the application startup and returns @true
        by default. In this case, wxWidgets ensures that the application is ran
        as a foreground, GUI application so that the user can interact with it
        normally, even if it is not bundled. If this is undesired, i.e. if the
        application doesn't need to be brought to the foreground, this method
        can be overridden to return @false.

        Notice that overriding it doesn't make any difference for the bundled
        applications which are always foreground unless @c LSBackgroundOnly key
        is specified in the @c Info.plist file.

        @onlyfor{wxosx}

        @since 3.0.1
    */
    virtual bool OSXIsGUIApplication();

    /**
        Enable the automatic tabbing features of macOS.

        This feature is native to the operating system. When it is enabled, macOS
        will automatically place windows inside tabs and show a tab bar in the
        application. Entries are also added to the View menu to show/hide the tab bar.

        @onlyfor{wxosx}

        @remarks Requires macOS 10.12+, does nothing under earlier OS versions.

        @since 3.1.4
    */
    void OSXEnableAutomaticTabbing(bool enable);

    ///@}

    /**
        @name MSW-specific functions
    */
    //@{

    /**
        Enable experimental dark mode support for MSW applications.

        This function uses @e undocumented, and unsupported by Microsoft,
        functions to enable dark mode support for the desktop applications
        under Windows 10 versions later than v1809 (which includes Windows 10
        LTSC 2019) and all Windows 11 versions. Please note that dark mode
        testing under versions of Windows earlier than 20H1 (i.e. v2004) has
        been limited, make sure to test your application especially carefully
        if you target these versions and want to enable dark mode support.

        Note that dark mode can also be enabled by setting the "msw.dark-mode"
        @ref wxSystemOptions "system option" via an environment variable from
        outside the application or by calling SetAppearance() with either
        `System` or `Dark` parameter.

        Known limitations of dark mode support include:

        - Anything based on `TaskDialog()` Win32 API doesn't support dark mode:
          wxMessageBox(), wxMessageDialog, wxRichMessageDialog, wxProgressDialog
          and simple (i.e., without hyperlink or licence) wxAboutBox(). Consider
          using generic versions (e.g. wxGenericMessageDialog or wxGenericProgressDialog)
          if dark mode support is more important than using the native dialog.
        - The following dialogs wrapping common windows dialogs don't support
          dark mode: wxColourDialog, wxFindReplaceDialog, wxFontDialog,
          wxPageSetupDialog, wxPrintDialog.
        - wxTimePickerCtrl, wxDatePickerCtrl and wxCalendarCtrl don't support dark mode
          and use the same (light) background as by default in it.
        - Toolbar items for which wxToolBar::SetDropdownMenu() was called
          don't draw the menu drop-down correctly, making it almost
          invisible.
        - Calling wxMenu::Break() or wxMenuItem::SetDisabledBitmap() will result
          in the menu being light.

        @param flags Can include @c wxApp::DarkMode_Always to force enabling
            dark mode for the application, even if the system doesn't use the
            dark mode by default. Otherwise dark mode is only used if it is the
            default mode for the applications on the current system.
        @param settings If specified, allows to customize dark mode appearance.
            Please see wxDarkModeSettings documentation for more information.

        @return @true if dark mode support was enabled, @false if it couldn't
            be done, most likely because the system doesn't support dark mode.

        @onlyfor{wxmsw}

        @since 3.3.0
     */
    bool
    MSWEnableDarkMode(int flags = 0, wxDarkModeSettings* settings = nullptr);

    //@}
};



// ============================================================================
// Global functions/macros
// ============================================================================


/** @addtogroup group_funcmacro_rtti */
///@{

/**
    This is used in headers to create a forward declaration of the ::wxGetApp()
    function implemented by wxIMPLEMENT_APP().

    It creates the declaration <tt>className& wxGetApp()</tt>
    (requires a final semicolon).

    @header{wx/app.h}

    Example:

    @code
    wxDECLARE_APP(MyApp);
    @endcode
*/
#define wxDECLARE_APP( className )

/**
    This macro defines the application entry point and tells wxWidgets which
    application class should be used.

    The two tasks performed by this macro can be done separately by using
    wxIMPLEMENT_APP_NO_MAIN() and wxIMPLEMENT_WXWIN_MAIN() macros, but in a
    typical GUI application it's simpler and more convenient to use this macro
    to do both together.

    The @a className passed to this macro must be a name of the class deriving
    from wxApp.

    Note that this macro requires a final semicolon.

    @header{wx/app.h}

    Example:

    @code
    wxIMPLEMENT_APP(MyApp);
    @endcode

    @see wxDECLARE_APP(), wxIMPLEMENT_APP_CONSOLE()
*/
#define wxIMPLEMENT_APP( className )

/**
    This macro defines the application entry point for non-GUI applications and
    tells wxWidgets which application class should be used.

    This macro is provided for symmetry with wxIMPLEMENT_APP() for the console
    (non-GUI) applications and is equivalent to using wxIMPLEMENT_APP_NO_MAIN()
    and wxIMPLEMENT_WXWIN_MAIN_CONSOLE().

    The @a className passed to this macro must be a name of the class deriving
    from wxApp.

    Note that this macro requires a final semicolon.

    @header{wx/app.h}

    Example:

    @code
    wxIMPLEMENT_APP_CONSOLE(MyApp);
    @endcode

    @see wxIMPLEMENT_APP()
*/
#define wxIMPLEMENT_APP_CONSOLE( className )

/**
    This macro defines the application entry point appropriate for the current
    platform.

    Note that usually wxIMPLEMENT_APP() is used instead of this macro.

    For most platforms, it defines @c main() function, but for GUI Windows
    applications, it defines @c WinMain() instead.

    In either case, the macro expansion includes the call to
    wxDISABLE_DEBUG_SUPPORT() which disables debugging code in release builds.
    If you don't use this macro, but define the entry point yourself, you
    probably want to call wxDISABLE_DEBUG_SUPPORT() explicitly.

    @header{wx/app.h}
 */
#define wxIMPLEMENT_WXWIN_MAIN

/**
    This macro defines the application entry point for console applications.

    This macro is provided mostly for symmetry with wxIMPLEMENT_WXWIN_MAIN()
    but is less useful, as it is also simple enough to define @c main()
    function directly.

    Please note, however, that this macro, as well as wxIMPLEMENT_APP_CONSOLE()
    which uses it, contains the call to wxDISABLE_DEBUG_SUPPORT() which
    disables debugging code in release builds and that if you don't use this
    macro, but define @c main() yourself, you probably want to call
    wxDISABLE_DEBUG_SUPPORT() from it explicitly.

    @header{wx/app.h}
 */
#define wxIMPLEMENT_WXWIN_MAIN_CONSOLE

///@}



/**
    The global pointer to the singleton wxApp object.

    This pointer can only be used in the GUI applications.

    @see wxAppConsole::GetInstance(), wxApp::GetGUIInstance()
*/
wxApp *wxTheApp;



/** @addtogroup group_funcmacro_appinitterm */
///@{

/**
    This function doesn't exist in wxWidgets but it is created by using the
    wxIMPLEMENT_APP() macro.

    Thus, before using it anywhere but in the same module where this macro is
    used, you must make it available using wxDECLARE_APP().

    The advantage of using this function compared to directly using the global
    ::wxTheApp pointer is that the latter is of type wxApp* and so wouldn't
    allow you to access the functions specific to your application class but
    not present in wxApp while wxGetApp() returns the object of the right type.

    @header{wx/app.h}
*/
wxAppDerivedClass& wxGetApp();

/**
    If @a doIt is @true, the fatal exceptions (also known as general protection
    faults under Windows or segmentation violations in the Unix world) will be
    caught and passed to wxApp::OnFatalException.

    By default, i.e. before this function is called, they will be handled in
    the normal way which usually just means that the application will be
    terminated. Calling wxHandleFatalExceptions() with @a doIt equal to @false
    will restore this default behaviour.

    Notice that this function is only available if @c wxUSE_ON_FATAL_EXCEPTION
    is 1 and under Windows platform this requires a compiler with support for
    SEH (structured exception handling) which currently means only Microsoft
    Visual C++.

    @header{wx/app.h}
*/
bool wxHandleFatalExceptions(bool doIt = true);

/**
    This function is used in wxBase only and only if you don't create
    wxApp object at all. In this case you must call it from your
    @c main() function before calling any other wxWidgets functions.

    If the function returns @false the initialization could not be performed,
    in this case the library cannot be used and wxUninitialize() shouldn't be
    called either.

    This function may be called several times but wxUninitialize() must be
    called for each successful call to this function.

    @header{wx/app.h}
*/
bool wxInitialize();

/**
    This function is for use in console (wxBase) programs only. It must be called
    once for each previous successful call to wxInitialize().

    @header{wx/app.h}
*/
void wxUninitialize();

/**
    This function wakes up the (internal and platform dependent) idle system,
    i.e. it will force the system to send an idle event even if the system
    currently @e is idle and thus would not send any idle event until after
    some other event would get sent. This is also useful for sending events
    between two threads and is used by the corresponding functions
    wxPostEvent() and wxEvtHandler::AddPendingEvent().

    @header{wx/app.h}
*/
void wxWakeUpIdle();

/**
    Calls wxAppConsole::Yield if there is an existing application object.

    Does nothing if there is no application (which typically only happens early
    during the program startup or late during its shutdown).

    @see wxEvtLoopBase::Yield()

    @header{wx/app.h}
*/
bool wxYield();

/**
    Calls wxApp::SafeYield.

    @header{wx/app.h}
*/
bool wxSafeYield(wxWindow* win = nullptr, bool onlyIfNeeded = false);

/**
    This function initializes wxWidgets in a platform-dependent way. Use this if you
    are not using the default wxWidgets entry code (e.g. main or WinMain).

    For example, you can initialize wxWidgets from a Microsoft Foundation Classes
    (MFC) application using this function.

    @note This overload of wxEntry is available under all platforms.

    @see wxEntryStart()

    @header{wx/app.h}
*/
int wxEntry(int& argc, wxChar** argv);

/**
    See wxEntry(int&,wxChar**) for more info about this function.

    Notice that the type of @a pCmdLine is @c char *, even in Unicode build.

    @remarks To clean up wxWidgets, call wxApp::OnExit followed by the static
             function wxApp::CleanUp. For example, if exiting from an MFC application
             that also uses wxWidgets:
             @code
             int CTheApp::ExitInstance()
             {
                // OnExit isn't called by CleanUp so must be called explicitly.
                wxTheApp->OnExit();
                wxApp::CleanUp();

                return CWinApp::ExitInstance();
             }
             @endcode

    @header{wx/app.h}
*/
int wxEntry(HINSTANCE hInstance,
            HINSTANCE hPrevInstance = nullptr,
            char* pCmdLine = nullptr,
            int nCmdShow = SW_SHOWNORMAL);

///@}



/** @addtogroup group_funcmacro_procctrl */
///@{

/**
    Exits application after calling wxApp::OnExit.

    Should only be used in an emergency: normally the top-level frame
    should be deleted (after deleting all other frames) to terminate the
    application. See wxCloseEvent and wxApp.

    @header{wx/app.h}
*/
void wxExit();

///@}

/** @addtogroup group_funcmacro_debug */
///@{

/**
    @def wxDISABLE_DEBUG_SUPPORT()

    Use this macro to disable all debugging code in release build when not
    using wxIMPLEMENT_APP().

    Currently this macro disables assert checking and debug and trace level
    logging messages in release build (i.e. when @c NDEBUG is defined). It is
    used by wxIMPLEMENT_APP() macro so you only need to use it explicitly if you
    don't use this macro but initialize wxWidgets directly (e.g. calls
    wxEntry() or wxEntryStart() itself).

    If you do not want to disable debugging code even in release build of your
    application, you can use wxSetDefaultAssertHandler() and
    wxLog::SetLogLevel() with @c wxLOG_Max parameter to enable assertions and
    debug logging respectively.

    @see wxDISABLE_ASSERTS_IN_RELEASE_BUILD(),
         wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD(),
         @ref overview_debugging

    @since 2.9.1

    @header{wx/app.h}
 */
#define wxDISABLE_DEBUG_SUPPORT() \
    wxDISABLE_ASSERTS_IN_RELEASE_BUILD(); \
    wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD()

///@}

