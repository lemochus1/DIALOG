#ifndef DIALOGAPI_H
#define DIALOGAPI_H

#include "server.h" // Original interface

//==================================================================================================
// Public API classes
//==================================================================================================

class DIALOGProcess;
class DIALOGNamedData;
class DIALOGCommandHandler;
class DIALOGProcedureProvider;
class DIALOGProcedureCaller;
class DIALOGServicePublisher;
class DIALOGServiceSubscriber;

//==================================================================================================
// Process
//==================================================================================================

/// Singleton representing key point for integration of the DIALOG library into Custom processes.
///
/// It should be safe to used it from any thread of the application. This requirement resuled
/// in the class not being based on QObject, as it would complicate things.
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

    /// Starts all DIALOG eventloops, connects to Control Server, triggers registrations and allows
    /// passing on messages.
    void start(QThread::Priority priority = QThread::NormalPriority);

    /// Returns whether it has been started.
    bool started() const;

    /// Kills the application.
    void stop();

    /// Sets expected Control Server Address.
    ///
    /// By default enviromental variables "DIALOG_CONTROL_SERVER_ADDRESS" and
    /// "DIALOG_CONTROL_SERVER_PORT" are used.
    /// Safe only before DIALOGProcess::start is called.
    void setControlServerAddress(const QString& address, quint16 port);

    /// Returns whether it is currently connected to Control Server.
    bool connectedToControlServer() const;

    /// Sets the Process name available to Control Server, otherwise the word "Unnamed" is used.
    ///
    /// Should be called before DIALOGProcess:start()
    bool setName(const QString& name);

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
    /// Returns instance that is expected to obtain the call return value.
    /// Validity check needed as it returns null reference in case of registration failure.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGServiceSubscriber> tryRequestService(const QString& name);

    /// Calls remote procedure specified by given instance.
    ///
    /// Unless connected to Control Server, it waits for a while before cancelling the attempt.
    /// Returns whether message could be sent.
    bool tryCallProcedure(QWeakPointer<DIALOGProcedureCaller> caller);

    /// Calls remote procedure of the given name with forwarded params.
    ///
    /// Unless connected to Control Server, it waits for a while before cancelling the attempt.
    /// Returns instance that is expected to obtain the call return value.
    /// Validity check needed as it returns null reference in case of missing connection
    /// to Control Server.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGProcedureCaller> tryCallProcedure(const QString& name,
                                                           const QByteArray& params);

    /// Calls remote procedure specified by given instance provided by named process.
    ///
    /// Unless connected to Control Server, it waits for a while before cancelling the attempt.
    /// Returns whether message could be sent.
    bool tryCallProcedure(QWeakPointer<DIALOGProcedureCaller> caller, const QString& processName);

    /// Calls remote procedure of the given name with forwarded params on named process.
    ///
    /// Unless connected to Control Server, it waits for a while before cancelling the attempt.
    /// Returns instance that is expected to obtain the call return value.
    /// Validity check needed as it returns null reference in case of missing connection
    /// to Control Server.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGProcedureCaller> tryCallProcedure(const QString& name,
                                                           const QByteArray& params,
                                                           const QString& processName);

    /// Calls remote procedure specified by given instance hosted on given address.
    ///
    /// Returns allways true as it cannot be validated synchronically.
    bool tryCallProcedure(QWeakPointer<DIALOGProcedureCaller> caller,
                          const QString& address,
                                quint16 port);

    /// Calls remote procedure of the given name with forwarded params hosted on given address.
    ///
    /// Returns valid instance that is expected to obtain the call return value.
    /// Keeps only weak referece to the created object.
    QSharedPointer<DIALOGProcedureCaller> tryCallProcedure(const QString& name,
                                                           const QByteArray& params,
                                                           const QString& address,
                                                                 quint16 port);

    /// Sends Command of given name with forwarded message to Control server for distribution.
    ///
    /// Unless connected to Control Server, it waits for a while before cancelling the attempt.
    /// Successful delivery is not controlled. Use a different format for reliable communication.
    /// Returns whether message could be sent.
    bool trySendCommand(const QString& name, const QByteArray& message);

    /// Sends Direct Command of given name with forwarded message to Control server for
    /// distribution among processes of given processName.
    ///
    /// Unless connected to Control Server, it waits for a while before cancelling the attempt.
    /// Successful delivery is not controlled. Use a different format for reliable communication.
    /// Returns whether message could be sent.
    bool trySendCommand(const QString& name, const QByteArray& message, const QString& processName);

    /// Sends Direct Command of given name with forwarded message to specified address.
    ///
    /// Successful delivery is not controlled. Use a different format for reliable communication.
    /// Returns allways true as it cannot be validated synchronically.
    bool trySendCommand(const QString& name,
                        const QByteArray& message,
                        const QString& address,
                              quint16 port);

private:
    DIALOGProcess();
    ~DIALOGProcess();

    // Trick to avoid duplicities.
    template<typename T>
    bool tryRegisterTypeImpl(QWeakPointer<T> handler,
                             DIALOGMessageHandlerType type,
                             const char* method);
    template<typename T>
    QSharedPointer<T> tryRegisterTypeImpl(const QString& name,
                                          DIALOGMessageHandlerType type,
                                          const char* method);

    /// Returns if started and connected. If not, it waits a while for a repair before return.
    ///
    /// Expected to be called in each method that triggers contacting Control Server.
    bool canContactControlServer();

public:
    DIALOGProcess(DIALOGProcess const&) = delete;
    void operator=(DIALOGProcess const&) = delete;

private:
    QString name;

    QPointer<Server> server;
    QPointer<SenderThread> sender;
    QPointer<ReceiverThread> receiver;

    ProcessAddress controlAddress;

    QMutex mutex;
};

//==================================================================================================
// Nested Receiver and Sender threads
//==================================================================================================

/// Following two classes represents adapter to the old interface of DIALOG library.
///
/// They should be eliminated eventually as this approach requires many unnecessary connections of
/// dummy slots to signals but now they are needed becouse of backwards compatibility.
/// Nested as they are not supposed to be used directly by the user.
/// Their rocedures are not documented as they are not a part of the API.
class DIALOGProcess::ReceiverThread: public VirtualThread
{
    Q_OBJECT

public:
    ~ReceiverThread();

    void run() override;

    // Overloaded for easier use with templates. Not really public anyway.
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGCommandHandler> commandHandler);
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGServiceSubscriber> serviceSubscriber);
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGServicePublisher> servicePublisher);
    bool tryRegisterMessageHandler(QWeakPointer<DIALOGProcedureProvider> procedureProvider);

    /// Registeres caller and returns its unique ID needed for addressing response.
    ///
    /// Returns -1 if registration failed.
    /// Requres different approach as it is the only non unique single-shot object.
    int tryRegisterProcedureCaller(QWeakPointer<DIALOGProcedureCaller> procedureCaller);

    void setSenderThread(QPointer<SenderThread> sender);

public slots:
    void messageReceivedSlot(QString senderAddress,
                             quint16 senderPort,
                             QByteArray *header,
                             QByteArray *message) override;
    void serverErrorSlot(QString error) override;

    void controlServerConnectedSlot();
    void controlServerUnavailableSlot();

    void lostServiceSubscriberSlot(const QString& name);
    void serviceUnavailableSlot(const QString& name);
    void procedureUnavailableSlot(const QString& name, int callerId);
    void newServiceSubscriberSlot(const QString& name);
    void serviceActivatedSlot(const QString& name);
    void serviceRegistrationFailedSlot(const QString& name);
    /// Triggers automatic unregistration
    ///
    /// Called whenever registered handler is deleted.
    void objectDeletedSlot();

private:
    // Trick to avoid duplicities.
    template<typename T>
    bool registerMessageHandlerImpl(QWeakPointer<T> handler,
                                    QMap<QString, QWeakPointer<T>>& map,
                                    DIALOGMessageHandlerType type);
    template<typename T>
    void removeInvalidPointers(QMap<QString, QWeakPointer<T>>& map,
                               const char* unregisterMethod);

    void removeInvalidProcedureCallers();
    bool trySetResponseAddress(const QString& senderAddress, int senderPort, int callerId);

    void commandMassageReceived(const QStringList& headerList, QByteArray* message);
    void serviceMessageReceived(const QStringList& headerList, QByteArray* message);
    void procedureMessageReceived(const QStringList& headerList,
                                  QByteArray* message,
                                  QString senderAddress,
                                  quint16 senderPort);

    QMap<QString, QWeakPointer<DIALOGCommandHandler>> commands;
    QMap<QString, QWeakPointer<DIALOGServiceSubscriber>> serviceSubscribers;
    QMap<QString, QWeakPointer<DIALOGServicePublisher>> servicePublishers;
    QMap<QString, QWeakPointer<DIALOGProcedureProvider>> procedureProviders;
    // Classifed by name and ID.
    QMap<QString, QMap<int, QWeakPointer<DIALOGProcedureCaller>>> procedureCallers;

    QList<QWeakPointer<DIALOGNamedData>> allHandlers;

    /// Needed for fast responses and error messages
    QPointer<SenderThread> senderThread;
    // Identifies messages so answers can be addressed.
    int messageId = 0;
    // Identifies caller objects so answers can be addressed.
    int callerId = 0;
    // Cache info about connection so it can determine a change.
    bool controlServerAvailable = false;
};

class DIALOGProcess::SenderThread: public VirtualThread
{
    Q_OBJECT

    struct ResponseAddress
    {
      QString address;
      quint16 port;
      int objectId;

      ResponseAddress() = default;
      ResponseAddress(const QString& address, quint16 port, int objectId)
          : address(address),
            port(port),
            objectId(objectId)
      {}
    };

public:
    ~SenderThread();

    void run() override;

    // Overloaded for easier use with templates. Not really public anyway.
    bool tryRegisterMessageSender(QWeakPointer<DIALOGCommandHandler> commandHandler);
    bool tryRegisterMessageSender(QWeakPointer<DIALOGServiceSubscriber> serviceSubscriber);
    bool tryRegisterMessageSender(QWeakPointer<DIALOGServicePublisher> servicePublisher);
    bool tryRegisterMessageSender(QWeakPointer<DIALOGProcedureProvider> procedureProvider);

    void setProcedureResponseAddress(int callId, int callerId, const QString &address, quint16 port);

public slots: // Just trigger signals for old API, it should be eliminated later.
    // Not overloaded because of problems with connecting to homonymous signals and consistency.
    void sendCommandSlot(const QString &name, const QByteArray &message);
    void sendCommandNameSlot(const QString& name,
                             const QByteArray &message,
                             const QString& processName);
    void sendCommandAddressSlot(const QString& name,
                                const QByteArray& message,
                                const QString& address,
                                quint16 port);
    void callProcedureSlot(const QString& name, const QByteArray& message, int callerId);
    void callProcedureNameSlot(const QString& name,
                               const QByteArray &message,
                               const QString& processName,
                               int callerId);
    void callProcedureAddressSlot(const QString& name,
                                  const QByteArray& message,
                                  const QString& address,
                                  quint16 port,
                                  int callerId);

    void sendProcedureReturnValueSlot(const QByteArray& data, int callId);
    void sendProcedureFailedSlot(const QByteArray& message, int callId);
    void sendParamsInvalidErrorSlot(int callId);

    void sendServiceDataSlot(const QByteArray &data);
    void sendServicePublisherMessageSlot(const QByteArray &message);
    void requestServiceUpdateSlot();
    void sendServiceSubscriberMessageSlot(const QByteArray &message);

private:
    // Trick to avoid duplicities.
    template<typename T>
    bool registerMessageSenderImpl(QWeakPointer<T> sender,
                                   QStringList& list,
                                   DIALOGMessageHandlerType type);
    template<typename T>
    QString getSenderName(QObject* obj);

    // Names to be registered ones the server is started.
    QStringList servicesToRegister;
    QStringList servicesToRequest;
    QStringList commandsToRegister;
    QStringList proceduresToRegister;
    // Maps messageId to response address.
    QMap<int, ResponseAddress> responseAddresses;
};

//==================================================================================================
// Base data class
//==================================================================================================

/// Contains functionality common to all supported communication formats.
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

private slots:// Receiving slots called by DIALOG internals.
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
    explicit DIALOGNamedData(const QString& name, QObject* parent=nullptr);
    virtual ~DIALOGNamedData() = default;

    /// Safely sets the data.
    void setData(const QByteArray& data);

    /// Safely sets error code.
    void setError(DIALOGErrorCode error);

    mutable QMutex mutex;

private:
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
class DIALOGCommandHandler: public DIALOGNamedData
{
    Q_OBJECT

public:
    explicit DIALOGCommandHandler(const QString& name, QObject* parent=nullptr);
    virtual ~DIALOGCommandHandler() = default;

private slots:// Receiving slots called by DIALOG internals.
    /// Sets data and emits commandReceivedSignal
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
    explicit DIALOGProcedureProvider(const QString &name, QObject* parent=nullptr);
    virtual ~DIALOGProcedureProvider() = default;

public slots: // Sending slots callable in the application.
    /// Saves data and emits callFinishedSignal
    void callFinishedSlot(const QByteArray& response, int callId);

    /// Sets error code and emits callFailedErrorSignal
    void callFailedErrorSlot(const QByteArray& message, int callId);

    /// Sets error code and emits paramsInvalidErrorSignal
    void paramsInvalidErrorSlot(int callId);

private slots: // Receiving slots called by DIALOG internals.
    /// Emits callRequestedSignal
    ///
    /// Called whenever this procedure call is requested.
    virtual void callRequestedSlot(const QByteArray& params, int callId);

signals:
    /// Indicates that this procedure has been called.
    void callRequestedSignal(const QByteArray& params, int callId);

    /// Triggers sending response to caller process.
    ///
    /// Mainly for internal purposes.
    void callFinishedSignal(const QByteArray& response, int callId);

    /// Triggers sending error message to caller process.
    ///
    /// Mainly for internal purposes.
    void callFailedErrorSignal(const QByteArray& message, int callId);

    /// Triggers sending error message to caller process.
    ///
    /// Mainly for internal purposes.
    void paramsInvalidErrorSignal(int callId);
};

/// Represents RPC caller object that is expected to obtain return value of called remote precedure.
///
/// Its life cycle is limited to just one call.
/// It can be both subclassed or used directly.
class DIALOGProcedureCaller: public DIALOGNamedData
{
    Q_OBJECT

public:
    explicit DIALOGProcedureCaller(const QString& name,
                                   const QByteArray& params,
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

    /// Sets timeout after which timeoutPassedErrorSignal is emmited unless response was received.
    void waitForTimeout(int sTimeout);

private slots: // Receiving slots called by DIALOG internals.
    /// Saves data and emits responseReceivedSignal.
    ///
    /// Called if excpected response is received.
    virtual void responseReceivedSlot(const QByteArray& response);

    /// Sets error code and emits callFailedErrorSignal.
    ///
    /// Called if error message is received.
    virtual void callFailedErrorSlot(const QByteArray& message);

    /// Sets error code and emits procedureUnavailableErrorSignal.
    ///
    /// Called if requested procedure is not registered.
    virtual void procedureUnavailableErrorSlot();

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
    void procedureUnavailableErrorSignal();

    /// Indicates that targeted procedure cannot be called with given params.
    void paramsInvalidErrorSignal();

    /// Indicates that set timeout passed and no responce has been received.
    void timeoutPassedErrorSignal();

private:
    void setHasResponse(bool response);

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
    explicit DIALOGServicePublisher(const QString& name, QObject* parent=nullptr);
    virtual ~DIALOGServicePublisher() = default;

    /// Returns whether it has at least one subscriber.
    bool hasSubscriber() const;

    /// Returns number of active subscribers.
    int getSubscriberCount() const;

public slots:// Sending slots callable in the application.
    /// Sets data and emits dataUpdatedSignal that causes sending updated data to all subscribers.
    void updateDataSlot(const QByteArray& data);

    /// Emits sendMessageSignal that causes sending given message to all subscribers.
    ///
    /// Allows different handling from data update in subscribers.
    void sendMessageToSubscribersSlot(const QByteArray& message);

private slots:// Receiving slots called by DIALOG internals.
    /// Emits signal dataUpdateRequestedSignal.
    ///
    /// Called whenever a subscriber requests data update.
    virtual void dataUpdateRequestedSlot();

    /// Emits messageReceivedSignal
    ///
    /// Called whenever subscriber sends a message.
    virtual void messageReceivedSlot(const QByteArray &message);

    /// Updates counter and emits subscriberConnectedSignal.
    ///
    /// Called whenever new subscriber is connected.
    void subscriberConnectedSlot();

    /// Updates counter and error code and emits subscriberLostSignal.
    ///
    /// Called whenever a subscriber is lost.
    void subscriberLostSlot();

    /// Sets error code and emits registrationFailedSignal
    ///
    /// Called in case of registration failure (E.g. other process already registered this Service).
    virtual void registrationFailedSlot();

signals:
    /// Emmited after data update to trigger sending messages to subscribers.
    ///
    /// Mainly for internal purposes.
    void dataUpdatedSignal(const QByteArray& data);

    ///Indicates that a subscriber sent a message
    void messageReceivedSignal(const QByteArray& message);

    /// Emmited after data update to trigger messages to subscribers.
    ///
    /// Mainly for internal purposes.
    void sendMessageToSubscribersSignal(const QByteArray& message);

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
    explicit DIALOGServiceSubscriber(const QString& name, QObject* parent=nullptr);
    virtual ~DIALOGServiceSubscriber() = default;

public slots:// Sending slots callable in the application.
    /// Emits sendMessageSignal.
    void sendMessageToProviderSlot(const QByteArray& message);

    /// Emits requestDataUpdateSignal.
    void requestDataUpdateSlot();

private slots:// Receiving slots called by DIALOG internals.
    /// Saves the data and emits dataUpdatedSignal.
    ///
    /// Called whenever provider updates the data.
    virtual void dataUpdatedSlot(const QByteArray& data);

    /// Emits messageReceivedSignal.
    ///
    /// Called whenever provider sends a message.
    virtual void messageReceivedSlot(const QByteArray& message);

    /// Sets error code and emits serviceUnavailableErrorSignal.
    ///
    /// Called whenever provider gets lost or after startup if the service is not registered.
    virtual void serviceUnavailableErrorSlot();

    /// Emits serviceActivatedSignal.
    ///
    /// Called whenever connection with publisher is established.
    virtual void serviceActivatedSlot();

signals:
    /// Triggers sending request for data update to the provider.
    ///
    /// Mainly for internal purposes.
    void requestDataUpdateSignal();

    /// Triggers sending message to the provider.
    ///
    /// Mainly for internal purposes.
    void sendMessageToProviderSignal(const QByteArray& message);

    /// Indicates that the provider updated its data.
    void dataUpdatedSignal(const QByteArray& data);

    /// Indicates that the provider sent a message.
    void messageReceivedSignal(const QByteArray& message);

    /// Indicates loss of connection to the Service provider or its non-existence during startup.
    void serviceUnavailableErrorSignal();

    /// Indicates new connection to the Service provider.
    void serviceActivatedSignal();
};

#endif // DIALOGAPI_H
