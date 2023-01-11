using System.Collections.Generic;

namespace Pyxis.WorldView.Studio
{
    /// <summary>
    /// Utility class that collect all CommandLine events from a CommandLineParser
    /// </summary>
    class CommandLineQueue
    {
        private readonly object m_lock = new object();
        private readonly Queue<CommandLineEventArgs> m_commandLineQueue = new Queue<CommandLineEventArgs>();

        /// <summary>
        /// Attaching a CommandLineQueue to a CommandLineParser.
        /// </summary>
        /// <param name="parser">CommandLineParser to listen to OnCommand events.</param>
        /// <returns>a Queue that will collect the commands from the CommandLineParser.</returns>
        public static CommandLineQueue Attach(CommandLineParser parser)
        {
            return new CommandLineQueue(parser);
        }

        private CommandLineQueue(CommandLineParser parser)
        {
            parser.OnCommand += ParserOnOnCommand;
        }

        private void ParserOnOnCommand(object sender, CommandLineEventArgs commandLineEventArgs)
        {
            lock (m_lock)
            {
                m_commandLineQueue.Enqueue(commandLineEventArgs); 
            }
        }

        /// <summary>
        /// Try to get the next command from the Queue.
        /// </summary>
        /// <returns>Next CommandLineEventArgs in Queue or null if queue is empty.</returns>
        public CommandLineEventArgs TryGetNextCommand()
        {
            lock (m_lock)
            {
                if (m_commandLineQueue.Count > 0)
                {
                    return m_commandLineQueue.Dequeue();
                }
                return null;
            }
        }
    }
}