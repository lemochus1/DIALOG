#ifndef DIALOGAPI_H
#define DIALOGAPI_H

#include <QObject>
#include <QHostInfo>
#include <QMutexLocker>

#include "server.h"
#include "define.h"
#include "processaddress.h"

class DIALOGServicePublisher;
class DIALOGServiceSubscriber;
class DIALOGNamedData;
class DIALOGCommandHandler;
class DIALOGProcedureCaller;
class DIALOGProcedureProvider;

//==================================================================================================
// DIALOG Process
//==================================================================================================

/// Singleton representing key point for integration of the DIALOG library into Custom processes.
///
/// It should be thread-safe and usable from any thread of the application. This requirement resuled
/// in the class not being based on QObject, as it would complicate the safety.
/// Most of the time it only invokes methods of specialized instances running in their own threads.
/// In that way this class hides the DIALOG threads model from the application.
/// The getter getServer can be used to access the original DIALOG API, but it is not recommended.
///
/// Standard usage scenario:
///     1. DIALOGProcess::setName()
///     (2. DIALOGProcess::setControlServerAddress())
///     3. All calls of DIALOGProcess::tryRegister/Request...()
///     4. DIALOGProcess::start()
///     5. Free use of DIALOGProcess::trySend...() or DIALOGProcess::tryCall..() methods
///     6. Whole process termination (possibly by DIALOGProcess::stop())
class DIALOGProcess
{
    class ReceiverThread;
    class SenderThread;

public:
    /// Returns the singleton instance.
    static DIALOGProcess &GetInstance();

    /// Starts all DIALOG eventloops, triggers registrations and allows passing on messages.
    void start(QThread::Priority priority = QThread::NormalPriority);

    /// Kills the application.
    void stop();

    /// Sets expected Control Server Address.
    ///
    /// By default enviromental variables "DIALOG_CONTROL_SERVER_ADDRESS" and
    /// "DIALOG_CONTROL_SERVER_PORT" are used.
    /// Safe only before DIALOGProcess::start is called.
    void setControlServerAddress(const QString& address, quint16 port);

    /// Sets the Process name available to Control Server, otherwise the word "Unnamed" is used.
    ///
    /// Should be called before DIALOGProcess:start()
    bool setName(const QString& nameInit);

    /// Returns Process name
    QString getName() const;

    /// Returns pointer to server object representing communication node of the system.
    ///
    /// Validity check needed as it is not initialized until it is started.
    QPointer<Server> getServer() const;

    /// Sends registration message to Control server and setups given object as a handler of
    /// received Command of its name.
    ///
    /// Returns whether registration succeeded (== unique and valid handler has been forwarded).
    bool tryRegisterCommand(QWeakPointer<DIALOGCommandHandler> command);

    /// Creates, registers and returns default version of Command handler for given name.
    ///
    /// Validity check needed as it returns null reference in case of registration failure.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGCommandHandler> tryRegisterCommand(const QString& name);

    /// Sends registration message to Control server and setups given object as a handler of
    /// remote calls.
    ///
    /// Returns whether registration succeeded (== unique and valid handler has been forwarded).
    bool tryRegisterProcedure(QWeakPointer<DIALOGProcedureProvider> procedure);

    /// Creates, registers and returns default version of Procedure provider for given name.
    ///
    /// Validity check needed as it returns null reference in case of registration failure.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGProcedureProvider> tryRegisterProcedure(const QString& name);

    /// Sends registration message to Control server and setups given object as a sender of
    /// regularly updated data.
    ///
    /// Returns whether registration succeeded (== unique and valid handler has been forwarded).
    bool tryRegisterService(QWeakPointer<DIALOGServicePublisher> provider);

    /// Creates, registers and returns default version of Service provider for given name.
    ///
    /// Validity check needed as it returns null reference in case of registration failure.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGServicePublisher> tryRegisterService(const QString& name);
    /// Sends request message to Control server and setups given object as a receiver of
    /// regularly updated data.
    ///
    /// Returns whether succeeded (== unique and valid handler has been forwarded).
    bool tryRequestService(QWeakPointer<DIALOGServiceSubscriber> subscriber);

    /// Creates, registers and returns default version of Service subscriber for given name.
    ///
    /// Validity check needed as it returns null reference in case of registration failure.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGServiceSubscriber> tryRequestService(const QString& name);

    /// Calls remote procedure of the given name with forwarded params.
    ///
    /// Validity check needed as it returns null reference in case of missing connection
    /// to Control Server.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGProcedureCaller> tryCallProcedure(const QString& name,
                                                           const QByteArray& params);
    //DIALOGProcedureCaller* callProcedure(QString name, QString process);
    //DIALOGProcedureCaller* callProcedure(QString name, QString url, int port);

    /// Sends Command of given name with forwarded message to Control server for distribution.
    ///
    /// Successful delivery is not controlled. Use a different format for reliable communication.
    /// Returns whether connected to Control server which is needed for sending the message.
    bool trySendCommand(const QString& name, const QByteArray& message);

    /// Sends Direct Command of given name with forwarded message to Control server for
    /// distribution among processes of given processName.
    ///
    /// Successful delivery is not controlled. Use a different format for reliable communication.
    /// Returns whether connected to Control server which is needed for sending the message.
    bool trySendDirectCommand(const QString& name,
                              const QByteArray& message,
                              const QString& processName);

    /// Sends Direct Command of given name with forwarded message to specified adress.
    ///
    /// Successful delivery is not controlled. Use a different format for reliable communication.
    /// Returns allways true as it cannot be validated synchronically.
    bool trySendDirectCommand(const QString& name,
                              const QByteArray& message,
                              const QString& address,
                              quint16 port);

private:
    DIALOGProcess();
    ~DIALOGProcess();

    /// Trick to avoid duplicities.
    template<typename T>
    bool tryRegisterTypeImpl(QWeakPointer<T> handler, DIALOGMessageHandlerType type);
    template<typename T>
    QSharedPointer<T> tryRegisterTypeImpl(const QString& name, DIALOGMessageHandlerType type);

public:
    DIALOGProcess(DIALOGProcess const&) = delete;
    void operator=(DIALOGProcess const&) = delete;

private:
    QString name;

    QPointer<Server> server;
    QPointer<SenderThread> sender;
    QPointer<ReceiverThread> receiver;

    ProcessAddress controlAddress;
};

//==================================================================================================
// Receiver and Sender threads
//==================================================================================================

/// Following two classes represents adapter to the old interface of DIALOG library.
///
/// They should be eliminated eventually but now they are needed becouse of backward compatibility.
/// Nested as they are not supposed to be used directly by the user.
/// Their rocedures are not documented as they are not a part of the API.
class DIALOGProcess::ReceiverThread: public VirtualThread
{
    Q_OBJECT

public:
    ~ReceiverThread();
    void run() override;

    /// Overloaded for easier use with templates. Not really public anyway.
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGCommandHandler> handler);
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGServiceSubscriber> handler);
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGServicePublisher> handler);
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGProcedureCaller> handler);
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGProcedureProvider> handler);

public slots:
    void messageReceivedSlot(QString senderName,
                             quint16 senderPort,
                             QByteArray *header,
                             QByteArray *message) override;

private:
    /// Trick to avoid duplicities.
    template<typename T>
    bool registerMessageHandlerImpl(QWeakPointer<T> handler,
                                    QMap<QString, QWeakPointer<T>>& map,
                                    DIALOGMessageHandlerType type);
    template<typename T>
    bool invokeMethod(QWeakPointer<T> handler, const char *method, QByteArray * message);

    QMap<QString, QWeakPointer<DIALOGCommandHandler>> commands;
    QMap<QString, QWeakPointer<DIALOGServiceSubscriber>> serviceSubscribers;
    QMap<QString, QWeakPointer<DIALOGProcedureCaller>> procedureCallers;
    QMap<QString, QWeakPointer<DIALOGProcedureProvider>> procedureProviders;
};

class DIALOGProcess::SenderThread: public VirtualThread
{
    Q_OBJECT

public:
    ~SenderThread();
    void run() override;

    /// Overloaded for easier use with templates. Not really public anyway.
    bool tryRegisterMessageSender(QWeakPointer<DIALOGCommandHandler> sender);
    bool tryRegisterMessageSender(QWeakPointer<DIALOGServiceSubscriber> sender);
    bool tryRegisterMessageSender(QWeakPointer<DIALOGServicePublisher> sender);
    bool tryRegisterMessageSender(QWeakPointer<DIALOGProcedureCaller> sender);
    bool tryRegisterMessageSender(QWeakPointer<DIALOGProcedureProvider> sender);

public slots:
    /// These slots just trigger signals for old API, it should be eliminated later.
    void sendCommandSlot(QString name, QByteArray* message);
    /// Not overloaded because of problems with connecting to homonymous signals and consistency.
    void sendDirectCommandNameSlot(QString name, QByteArray* message, QString processName);
    void sendDirectCommandAddressSlot(QString name,
                                      QByteArray* message,
                                      QString address,
                                      quint16 port);
    void sendServiceSlot(QString name, QByteArray message);
    void callProcedureSlot(QString name, QByteArray message);
    void sendProcedureReturnValueSlot(QString name, QByteArray data, QString address, quint16 port);

private:
    /// Trick to avoid duplicities.
    template<typename T>
    bool registerMessageSenderImpl(QWeakPointer<T> sender,
                                   QStringList& list,
                                   DIALOGMessageHandlerType type);

    /// Names to be registered ones the server is started.
    QStringList servicesToRegister;
    QStringList servicesToRequest;
    QStringList commandsToRegister;
    QStringList proceduresToRegister;
};

//==================================================================================================
// Help classes
//==================================================================================================

/// Class containing functionality common to all supported communication formats.
///
/// There should be no need to use it directly. Might for polymorphism.
class DIALOGNamedData: public QObject
{
    Q_OBJECT

public:
    /// Returns name of the data.
    QString getName() const;

    /// Returns data as byte array.
    QByteArray getData() const;

    /// Returns last occurred error code.
    DIALOGErrorCode getError() const;

    /// Returns whether an error occurred.
    bool errorOccurred() const;

private slots:// Called by DIALOG internals.
    /// Emits controlServerConnectedSignal.
    ///
    /// Called whenever its process gets connected to Control Server.
    virtual void controlServerConnectedSlot();

    /// Sets error code and emits controlServerUnavailableErrorSignal.
    ///
    /// Called whenever its process loses connection to Control Server.
    virtual void controlServerUnavailableErrorSlot();

signals:
    /// Indicates new connection to Control Server.
    void controlServerConnectedSignal();

    /// Indicates loss of connection to Control Server.
    void controlServerUnavailableErrorSignal();

protected:
    explicit DIALOGNamedData(const QString& nameInit, QObject* parent=nullptr);
    virtual ~DIALOGNamedData() = default;

    mutable QMutex mutex;

    QByteArray data;
    QString name;
    DIALOGErrorCode error;
};

//==================================================================================================
// Command
//==================================================================================================

/// Represents simple handler of Commands
///
/// The only completely passive handler. E.g. it does not send the receiving comfirmation.
/// Can be both used directly and subclassed.
class DIALOGCommandHandler : public DIALOGNamedData
{
    Q_OBJECT

public:
    explicit DIALOGCommandHandler(const QString& nameInit, QObject* parent=nullptr);
    virtual ~DIALOGCommandHandler() = default;

private slots:// Called by DIALOG internals.
    /// Emits commandReceivedSignal
    ///
    /// Called whenewer this Command is received.
    virtual void commandReceivedSlot(const QByteArray& message);

signals:
    /// Indicates that the Command was received.
    void commandReceivedSignal(const QByteArray& message);
};

//==================================================================================================
// Procedure
//==================================================================================================

/// Following classes represents simple implementation of RPC based on messages exchange.
/// Unlike Command they arrange 1-to-1 communication and a response is expected from asked process.

/// Encapsulates remotely callable method.
///
/// Designed to be subclassed by domain-specific descendants.
/// It should live in a special thread with eventloop and communicate safely by signals and slots.
class DIALOGProcedureProvider: public DIALOGNamedData
{
    Q_OBJECT

public:
    explicit DIALOGProcedureProvider(const QString &nameInit, QObject* parent=nullptr);
    virtual ~DIALOGProcedureProvider() = default;

public slots: // Callable in the application.
    /// Saves data and emits callFinishedSignal
    void callFinishedSlot(const QByteArray& response);

    /// Sets error code and emits callFailedErrorSignal
    void callFailedErrorSlot(const QByteArray& message);

    /// Sets error code and emits paramsInvalidErrorSignal
    void paramsInvalidErrorSlot();

private slots: // Called by DIALOG internals.
    /// Emits callRequestedSignal
    ///
    /// Called whenever this procedure call is requested.
    virtual void callRequestedSlot(const QByteArray& params);

signals:
    /// Indicates that this procedure has been called.
    void callRequestedSignal(const QByteArray& params);
    /// Triggers sending response to caller process.
    ///
    /// Mainly for internal purposes.
    void callFinishedSignal(const QByteArray& response);
    /// Triggers sending error message to caller process.
    ///
    /// Mainly for internal purposes.
    void callFailedErrorSignal(const QByteArray& message);
    /// Triggers sending error message to caller process.
    ///
    /// Mainly for internal purposes.
    void paramsInvalidErrorSignal();
};

/// Represents RPC caller object that is expected to obtain return value of called remote precedure.
///
/// Its life cycle is limited to just one call.
/// It can be both subclassed or used directly.
class DIALOGProcedureCaller: public DIALOGNamedData
{
    Q_OBJECT

public:
    explicit DIALOGProcedureCaller(const QString& nameInit,
                                   const QByteArray& paramsInit,
                                   QObject* parent=nullptr);
    virtual ~DIALOGProcedureCaller() = default;

    /// Waits for responseReceivedSignal or for given timeout to pass.
    ///
    /// Returns obtained data or in case of timeout an empty array.
    QByteArray waitForResponse(bool &ok, int sTimeout=10);

    /// Returns whether response has already been received.
    bool responseReceived() const;

    /// Returns params that were used while calling the remote function.
    QByteArray getParams() const;

    /// Sets timeout after which emmits timeoutPassedErrorSlot unless response is received sooner.
    void setTimeout(int sTimeout);

public slots: // Callable in the application.
    /// Emits tryAnotherCallSignal
    void tryAnotherCallSlot();

    /// Emits tryAnotherCallNameSignal
    void tryAnotherCallNameSlot(const QString& providerName);

    /// Emits tryANnotherCallAddressSignal
    void tryAnotherCallAddressSlot(const QString& providerAddress, quint16 providerPort);

private slots: // Called by DIALOG internals.
    /// Saves data and emits responseReceivedSignal.
    ///
    /// Called if excpected response is received.
    virtual void responseReceivedSlot(const QByteArray& response);

    /// Sets error code and emits callFailedErrorSignal.
    ///
    /// Called if error message is received.
    virtual void callFailedErrorSlot(const QByteArray& message);

    /// Sets error code and emits procedureUnevailableErrorSignal.
    ///
    /// Called if requested procedure is not registered.
    virtual void procedureUnevailableErrorSlot();

    /// Sets error code and emits paramsInvalidErrorSignal.
    ///
    /// Called if used params were considered invalid by procedure provider.
    virtual void paramsInvalidErrorSlot();

    /// Checks whether it already has a response. Otherwise it emits timeoutPassedErrorSignal.
    ///
    /// Called after timeout set by setTimeout().
    virtual void timeoutPassedErrorSlot();

signals:
    /// Indicates that expected data has been received.
    void responseReceivedSignal(const QByteArray& response);

    /// Indicates that the call failed and provider was able to deliver why.
    void callFailedErrorSignal(const QByteArray& message);

    /// Indicates that wanted procedure is not registered on targeted process or Control Server.
    void procedureUnevailableErrorSignal();

    /// Indicates that targeted procedure cannot be called with given params.
    void paramsInvalidErrorSignal();

    /// Indicates that set timeout passed and no responce has been received.
    void timeoutPassedErrorSignal();

    /// Triggeres another call of the remote procedure.
    ///
    /// Mainly for internal purposes.
    void tryAnotherCallSignal();

    /// Triggers another call of the remote procedure provided by named process.
    ///
    /// Mainly for internal purposes.
    void tryAnotherCallNameSignal(const QString& providerName);

    /// Triggers anoother call of the remote procedure provided on given address.
    ///
    /// Mainly for internal purposes.
    void tryANnotherCallAddressSignal(const QString& providerAddress, quint16 providerPort);

private:
    bool hasResponse  = false;

    QByteArray params;
};

//==================================================================================================
// Service
//==================================================================================================

/// Following classes allows implementation of Provider/Subscriber pattern represented by Service
/// in DIALOG.
/// Unlike Procedure and Command these classes establish long-term 1-to-m communication.
/// Both should live in a special thread with eventloop and communicate safely by signals and slots.

/// Publishing class allows serving up-to-date data to other processes.
///
/// Can be used directly.
class DIALOGServicePublisher: public DIALOGNamedData
{
    Q_OBJECT
    
public:
    explicit DIALOGServicePublisher(const QString& nameInit, QObject* parent=nullptr);
    virtual ~DIALOGServicePublisher() = default;

    /// Returns whether it has at least one subscriber.
    bool hasSubscriber() const;

    /// Returns number of active subscribers.
    int getSubscriberCount() const;

public slots:// Callable in the application.
    /// Sets data and emits dataUpdatedSignal that causes sending updated data to all subscribers.
    void updateDataSlot(const QByteArray& dataInit);

    /// Emits sendMessageSignal that causes sending given message to all subscribers.
    ///
    /// Allows different handling from data update by subscribers.
    void sendMessageToSubscribersSlot(const QByteArray& message);

private slots:// Called by DIALOG internals.
    /// Emits signal dataUpdateRequestedSignal.
    ///
    /// Called whenever a subscriber requests data update.
    virtual void dataUpdateRequestedSlot();

    /// Updates counter and emits subscriberConnectedSignal.
    ///
    /// Called whenever new subscriber is connected.
    virtual void subscriberConnectedSlot();

    /// Updates counter and error code and emits subscriberLostSignal.
    ///
    /// Called whenever a subscriber is lost.
    virtual void subscriberLostSlot();

    /// Sets error code and emits registrationFailedSignal
    ///
    /// Called in case of registration failure (E.g. other process already registered this Service).
    virtual void registrationFailedSlot();

signals:
    /// Emmited after data update to trigger sending messages to subscribers.
    ///
    /// Mainly for internal purposes.
    void dataUpdatedSignal(const QString& name, const QByteArray& data);

    /// Emmited after data update to trigger messages to subscribers.
    ///
    /// Mainly for internal purposes.
    void sendMessageSignal(const QString& name, const QByteArray& message);

    /// Indicates subscribers need for data update.
    void dataUpdateRequestedSignal();

    /// Indicates new subscriber
    void subscriberConnectedSignal();

    /// Indicates loss of a subscriber
    void subscriberLostSignal();

    /// Indicates that the service could not be registered and this object is no longer useful.
    void registrationFailedSignal();

private:
    int subscriberCount;
};

/// Subscribing class enables consumption of data obtained by different process.
///
/// Designed to be derived by domain-specific descendants.
class DIALOGServiceSubscriber: public DIALOGNamedData
{
    Q_OBJECT
    
public:
    explicit DIALOGServiceSubscriber(const QString& nameInit, QObject* parent=nullptr);
    virtual ~DIALOGServiceSubscriber() = default;

public slots:// Callable in the application.
    /// Emits sendMessageSignal.
    void sendMessageToProviderSlot(const QByteArray& message);

    /// Emits requestDataUpdateSignal.
    void requestDataUpdateSlot();

private slots:// Called by DIALOG internals.
    /// Saves the data and emits dataUpdatedSignal.
    ///
    /// Called whenever provider updates the data.
    virtual void dataUpdatedSlot(const QByteArray& dataInit);

    /// Emits messageReceivedSignal.
    ///
    /// Called whenever provider sends a message.
    virtual void messageReceivedSlot(const QByteArray& message);

    /// Sets error code and emits serviceUnavailableErrorSignal.
    ///
    /// Called whenever provider gets lost or after startup if the service is not registered.
    virtual void serviceUnavailableSlot();

    /// Emits serviceActivatedSignal.
    ///
    /// Called whenever service is put into operation.
    virtual void serviceActivatedSlot();

signals:
    /// Triggers sending request for data update to the provider.
    ///
    /// Mainly for internal purposes.
    void requestDataUpdateSignal();

    /// Triggers sending message to the provider.
    ///
    /// Mainly for internal purposes.
    void sendMessageSignal(const QByteArray& message);

    /// Indicates that the provider updated its data.
    void dataUpdatedSignal(const QByteArray& data);

    /// Indicates that the provider sent a message.
    void messageReceivedSignal(const QByteArray& message);

    /// Indicates new connection to the Service provider.
    void serviceActivatedSignal();

    /// Indicates loss of connection to the Service provider or its non-existence during startup.
    void serviceUnavailableErrorSignal();
};

#endif // DIALOGAPI_H
