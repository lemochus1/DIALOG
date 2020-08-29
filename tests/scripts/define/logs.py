#===================================================================================================
# Common
#===================================================================================================

TERMINAL_SEPARATOR = "-----------------------------------------"
LOG_SEPARATOR      = TERMINAL_SEPARATOR + TERMINAL_SEPARATOR + "\n"

TAB = "--> "

#===================================================================================================
# Parsing
#===================================================================================================

# Errors
WARNING  = "WARNING:"
CRITICAL = "CRITICAL:"
FATAL    = "FATAL:"

NO_HEART_BEAT                   = "crashed (No HeartBeats have been received)."
CONNECT_REFUSED                 = "The connection was refused by the"
CONNECT_REFUSED_DEFAULT_HANDLER = "Server error received: CONNECTION_ERROR|CONNECTION_REFUSED_ERROR"
CONNECT_FAILED                  = "Connection to control server failed."
NOT_REGISTERED                  = "is not registered on CommunicationControlSever."
NO_SENDER                       = "has no sender"
LOST_SENDER                     = "Lost sender of service Service"
ERROR_MESSAGE_PARTS = [WARNING, CRITICAL, FATAL, NO_HEART_BEAT,
                       CONNECT_REFUSED, CONNECT_FAILED, NOT_REGISTERED, NO_SENDER]

# Standard messages
START_MESSAGE_START = "The server is running on IP:"
CONNECTED_MESSAGE   = "connected to CommunicationControlServer"
KILLED_MESSAGE      = "Killed during cleanup.";

CONTROL_CONNECT_MESSAGE = "is successfully connected to CommunicationControlServer."
REGISTERED_MESSAGE      = "has been registered on CommunicationControlServer."
RECEIVED_MESSAGE        = "has been received on CommunicationControlServer and sent to"
SUBSCRIBED_MESSAGE      = "has been subscribed by"

INFO_ABOUT_SERVICE_MESSAGE = "Info about service"
NEW_SENDER_MESSAGE         = "New sender of service"

STANDARD_MESSAGE_PARTS = [START_MESSAGE_START, CONNECTED_MESSAGE, KILLED_MESSAGE,
                          CONTROL_CONNECT_MESSAGE, REGISTERED_MESSAGE, RECEIVED_MESSAGE]

#===================================================================================================
# Evaluation
#===================================================================================================

NO_CONNECTION_FORMAT_MESSAGE  = "Process {} was not connected to Control Server."
UNEXPECTED_LOG_FORMAT_MESSAGE = "Unexpected log on {}:"
ERROR_OCCURED_FORMAT_MESSAGE  = "Error occurred on {}:"
NO_REGISTER_FORMAT_MESSAGE    = "Handler {} did not register anything."
NO_CONSISTENT_FORMAT_MESSAGE  = " {} did not receive {} messages."
DEMAGED_MESSAGE_MESSAGE       = "Some message was delivered demaged."

PASSED_MESSAGE        = "Passed!"
FAILED_MESSAGE        = "Failed!"
NO_EVALUATION_MESSAGE = "No evaluation!"
