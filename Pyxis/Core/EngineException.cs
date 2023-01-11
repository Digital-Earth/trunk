using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core
{
    /// <summary>
    /// The exception that is thrown when an error occurs during the operation of a Pyxis.Core.Engine.
    /// </summary>
    public class EngineException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the Pyxis.Core.EngineException class.
        /// </summary>
        public EngineException()
        {
        }

        /// <summary>
        /// Initializes a new instance of the Pyxis.Core.EngineException class with a specified error message.
        /// </summary>
        /// <param name="message">A message that describes the error.</param>
        public EngineException(string message)
            : base(message)
        {
        }

        /// <summary>
        /// Initializes a new instance of the Pyxis.Core.EngineException class with a specified error message and a reference to the inner exception that is the cause of this exception.
        /// </summary>
        /// <param name="message">The error message that explains the reason for the exception.</param>
        /// <param name="innerException">The exception that is the cause of the current exception. If the innerException parameter is not a null reference, the current exception is raised in a catch block that handles the inner exception.</param>
        public EngineException(string message, Exception innerException)
            : base(message,innerException)
        {
        }
    }
}
