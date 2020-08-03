#include "daqdebugger.h"
#include <tr1/memory>

#define SUSPEND_SIG SIGUSR1

QString DAQDebugger::process = "";
QString DAQDebugger::processName = "";
QMutex DAQDebugger::mutex;
QMap<Qt::HANDLE, QThreadDAQDebugger*> DAQDebugger::threads;
Qt::HANDLE DAQDebugger::crashedThread = 0;
FILE* DAQDebugger::pFile = NULL;
quint32 DAQDebugger::threadCounter = 0;
sigset_t DAQDebugger::sigActionThreadControlSignalMask;
struct sigaction DAQDebugger::sigActionThreadControlSignal;
struct sigaction DAQDebugger::sigActionThreadControlSuspend;

QString DAQDebugger::readSystemCommand(QString command)
{
    char buffer[128];
    QString result = "";
    std::tr1::shared_ptr<FILE> pipe(popen(command.toStdString().c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get()))
    {
        if (fgets(buffer, 128, pipe.get()) != NULL)
        {
            result.append(buffer);
        }
    }

    return result;
}

void DAQDebugger::printStackTrace(FILE *out = stderr, unsigned int max_frames = 63)
{
    QByteArray threadID;
    threadID.append("0x");
    threadID.append(QString::number(quint64(QThreadDAQDebugger::currentThreadId()), 16));
    if(crashedThread == QThreadDAQDebugger::currentThreadId())
        fprintf(out, "\nStack trace of thread #%d: %s(%s) (CRASHED THREAD)\n", threadCounter, QThreadDAQDebugger::currentThread()->metaObject()->className(), threadID.data());
    else
        fprintf(out, "\nStack trace of thread #%d: %s(%s)\n", threadCounter, QThreadDAQDebugger::currentThread()->metaObject()->className(), threadID.data());

    // storage array for stack trace address data
    void* addrlist[max_frames + 1];

    // retrieve current stack addresses
    unsigned int addrlen = backtrace( addrlist, sizeof( addrlist ) / sizeof( void* ));

    if ( addrlen == 0 )
    {
        fprintf( out, "  \n" );
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // Actually it will be ## program address function + offset
    // this array must be free()-ed
    char** symbollist = backtrace_symbols( addrlist, addrlen );

    size_t funcnamesize = 1024;
    char funcname[1024];

    QString command = "addr2line -e " + DAQDebugger::process + " ";
    QString lineNumber;
    QString sourceCodeline;
    QString leftBracket = "[";
    QString rightBracket = "]";
    QString decodedSourceCodeLine = "";
    int leftBracketPosition;
    int rightBracketPosition;

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for ( unsigned int i = 1; i < addrlen; i++ )
    {
        sourceCodeline = QString(symbollist[i]);
        leftBracketPosition = sourceCodeline.indexOf(leftBracket);
        rightBracketPosition = sourceCodeline.indexOf(rightBracket);
        lineNumber = sourceCodeline.mid(leftBracketPosition + 1, rightBracketPosition - leftBracketPosition - 1);

        decodedSourceCodeLine = readSystemCommand(command + lineNumber);

        char* begin_name   = NULL;
        char* begin_offset = NULL;
        char* end_offset   = NULL;

        // find parentheses and +address offset surrounding the mangled name
        // ./module(function+0x15c) [0x8048a6d]
        for ( char *p = symbollist[i]; *p; ++p )
        {
            if ( *p == '(' )
                begin_name = p;
            else if ( *p == '+' )
                begin_offset = p;
            else if ( *p == ')' && ( begin_offset || begin_name ))
                end_offset = p;
        }

        if ( begin_name && end_offset && ( begin_name < end_offset ))
        {
            *begin_name++   = '\0';
            *end_offset++   = '\0';
            if ( begin_offset )
                *begin_offset++ = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status = 0;
            char* ret = abi::__cxa_demangle( begin_name, funcname,
                                             &funcnamesize, &status );
            char* fname = begin_name;
            if ( status == 0 )
                fname = ret;

            if ( begin_offset )
            {
                fprintf( out, "  %-30s ( %-40s  + %-6s) %s at line %s", symbollist[i], fname, begin_offset, end_offset, decodedSourceCodeLine.toStdString().c_str() );
            } else {
                fprintf( out, "  %-30s ( %-40s    %-6s) %s at line %s", symbollist[i], fname, "", end_offset, decodedSourceCodeLine.toStdString().c_str() );
            }
        } else {
            // couldn't parse the line? print the whole line.
            fprintf(out, "  %-40s", symbollist[i]);
        }
    }

    free(symbollist);
}

void DAQDebugger::getThreadStackTrace(int signum)
{
    if(threadCounter > 0)
    {
        if(crashedThread == QThreadDAQDebugger::currentThreadId())
            return;
    }
    // Dump a stack trace.
    // This is the function we will be implementing next.
    printStackTrace(pFile);

    threadCounter++;

    if(crashedThread != QThreadDAQDebugger::currentThreadId())
    {
        if(pthread_kill((pthread_t)crashedThread, signum) != 0)
            return;
        if(pthread_kill((pthread_t)QThreadDAQDebugger::currentThreadId(), SUSPEND_SIG) != 0)
            return;
    }
}

void DAQDebugger::abortHandler(int signum)
{
    mutex.lock();
    crashedThread = QThreadDAQDebugger::currentThreadId();

    sigfillset(&sigActionThreadControlSignalMask);
    sigdelset(&sigActionThreadControlSignalMask, signum);

    /* SUSPEND THREADS */
    foreach(Qt::HANDLE key, threads.keys())
    {
        if(key != crashedThread)
        {
            if(pthread_kill((pthread_t)key, SUSPEND_SIG) != 0)
                return;
        }
    }

    /* REGISTER SIGNAL ACTION FOR THREADS */
    sigfillset(&sigActionThreadControlSignal.sa_mask);
    sigdelset(&sigActionThreadControlSignal.sa_mask, signum);

    sigActionThreadControlSignal.sa_flags = 0;
    sigActionThreadControlSignal.sa_handler = getThreadStackTrace;
    sigaction(signum, &sigActionThreadControlSignal, NULL);

    /* OPEN FILE */
    if(pFile == NULL)
    {
        QDateTime now = QDateTime::currentDateTime();
        QString address = QString("/tmp/%1-%2%3%4-%5-%6-%7.report").arg(DAQDebugger::processName).arg(now.date().year()).arg(now.date().month(), 2, 10, QChar('0')).arg(now.date().day(), 2, 10, QChar('0')).arg(now.time().hour(), 2, 10, QChar('0')).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0'));
        pFile = fopen (address.toStdString().c_str() , "w");

        readSystemCommand(QString("gcore -o " + address + ".core " + QString::number(QCoreApplication::applicationPid())));

        // associate each signal with a signal name string.
        const char* name = NULL;
        switch( signum )
        {
        case SIGABRT: name = "SIGABRT";  break;
        case SIGSEGV: name = "SIGSEGV";  break;
        case SIGBUS:  name = "SIGBUS";   break;
        case SIGILL:  name = "SIGILL";   break;
        case SIGFPE:  name = "SIGFPE";   break;
        }

        // Notify the user which signal was caught. We use printf, because this is the
        // most basic output function. Once you get a crash, it is possible that more
        // complex output systems like streams and the like may be corrupted. So we
        // make the most basic call possible to the lowest level, most
        // standard print function.
        fprintf( pFile, "Datetime: %s\n", now.toString().toStdString().c_str() );
        if ( name )
            fprintf( pFile, "Caught signal %d (%s)\n", signum, name );
        else
            fprintf( pFile, "Caught signal %d\n", signum );
    }

    // CRASHED THREAD
    getThreadStackTrace(signum);

    /* SEND SIGNAL TO ALL THREADS */
    foreach(Qt::HANDLE key, threads.keys())
    {
        if(key != crashedThread)
        {
            if(pthread_kill((pthread_t)key, signum) != 0)
                return;

            /* SUSPEND CRASHED THREAD */
            sigsuspend(&sigActionThreadControlSignalMask);
        }
    }

    /*fprintf(pFile, "\nList of open files by process:\n");
    fprintf(pFile, "%s", readSystemCommand(QString("lsof -p " + QString::number(QCoreApplication::applicationPid()))).toStdString().c_str());*/
    /*fprintf(pFile, "\n\n");
    fprintf(pFile, "%s", readSystemCommand(QString("cat /proc/" + QString::number(QCoreApplication::applicationPid())) + "/maps").toStdString().c_str());*/

    /* CLOSE FILE */
    fclose (pFile);

    mutex.unlock();

    // If you caught one of the above signals, it is likely you just
    // want to quit your program right now.
    exit( signum );
}

void DAQDebugger::suspendHandler(int signum)
{
    sigsuspend(&sigActionThreadControlSignalMask);
}

void DAQDebugger::abortHandler2(int signum)
{
    std::cout << "Datetime: " << QDateTime::currentDateTime().toString().toStdString() << std::endl;
    std::cout << "Terminating signal: " << signum << std::endl;

    exit( signum );
}

void DAQDebugger::init(QString processInit, QString processNameInit)
{
    if (processInit.contains("/"))
        DAQDebugger::process = processInit;
    else
        DAQDebugger::process = readSystemCommand("which " + processInit).trimmed();

    if (processNameInit.size() == 0)
        DAQDebugger::processName = process.split("/").last();
    else
        DAQDebugger::processName = processNameInit;

    signal( SIGABRT, abortHandler );
    signal( SIGSEGV, abortHandler );
    signal( SIGILL,  abortHandler );
    signal( SIGFPE,  abortHandler );

    /* ALL OTHER SIGNALS */
    signal( SIGHUP, abortHandler2 );
    signal( SIGQUIT, abortHandler2 );
    //signal( SIGTRAP, abortHandler2 );
    signal( SIGBUS, abortHandler2 );
    //signal( SIGKILL, abortHandler2 );
    //signal( SIGUSR1, abortHandler2 );
    //signal( SIGUSR2, abortHandler2 );
    //signal( SIGPIPE, abortHandler2 );
    //signal( SIGALRM, abortHandler2 );
    signal( SIGTERM, abortHandler2 );
    //signal( SIGSTKFLT, abortHandler2 );
    //signal( SIGCHLD, abortHandler2 );
    //signal( SIGCONT, abortHandler2 );
    signal( SIGSTOP, abortHandler2 );
    signal( SIGTSTP, abortHandler2 );
    signal( SIGTTIN, abortHandler2 );
    signal( SIGTTOU, abortHandler2 );
    //signal( SIGURG, abortHandler2 );
    signal( SIGXCPU, abortHandler2 );
    signal( SIGXFSZ, abortHandler2 );
    //signal( SIGVTALRM, abortHandler2 );
    //signal( SIGPROF, abortHandler2 );
    //signal( SIGWINCH, abortHandler2 );
    //signal( SIGIO, abortHandler2 );
    //signal( SIGPWR, abortHandler2 );
    //signal( SIGSYS, abortHandler2 );

    /* SUSPEND THREADS */
    sigfillset(&sigActionThreadControlSuspend.sa_mask);
    sigdelset(&sigActionThreadControlSuspend.sa_mask, SUSPEND_SIG);

    sigActionThreadControlSuspend.sa_flags = 0;
    sigActionThreadControlSuspend.sa_handler = suspendHandler;
    sigaction(SUSPEND_SIG, &sigActionThreadControlSuspend, NULL);

    /* ADD MAIN THREAD */
    addThreadSlot(QThreadDAQDebugger::currentThread());
}

void DAQDebugger::addThreadSlot(QThreadDAQDebugger* thread)
{
    mutex.lock();
    threads[QThreadDAQDebugger::currentThreadId()] = thread;
    mutex.unlock();
}

void DAQDebugger::removeThreadSlot(QThreadDAQDebugger* thread)
{
    mutex.lock();
    foreach(Qt::HANDLE key, threads.keys())
    {
        if(threads[key] == thread)
        {
            threads.remove(key);
            break;
        }
    }
    mutex.unlock();
}
