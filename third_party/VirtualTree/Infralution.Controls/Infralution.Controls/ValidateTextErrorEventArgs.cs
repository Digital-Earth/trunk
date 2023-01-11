#region File Header
//
//      FILE:   ValidateTextErrorEventArgs.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
namespace Infralution.Controls
{

    /// <summary>
    /// Allows ValidateTextError event clients to handle errors encountered while
    /// converting text to an object value.
    /// </summary>
    /// <seealso cref="UniversalEditBox.ValidateTextError"/>
    public class ValidateTextErrorEventArgs : System.EventArgs
    {
        #region Member Variables

        private string      _text;
        private Exception   _exception;
        private bool        _handled = false;
        private bool        _resetText = false;
 
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="text">The text being validated</param>
        /// <param name="exception">The exception that occurred while converting</param>
        public ValidateTextErrorEventArgs(string text, Exception exception)
        {
            _text = text;
            _exception = exception;
        }

        /// <summary>
        /// The text being validated
        /// </summary>
        public string Text
        {
            get { return _text; }
        }

        /// <summary>
        /// The exception that occurred during conversion
        /// </summary>
        public Exception Exception
        {
            get { return _exception; }
        }

        /// <summary>
        /// Set/Get whether the event is handled by the client.
        /// </summary>
        /// <remarks>
        /// If Handled is set to true then the control will not display
        /// the standard error message.    
        /// </remarks>
        public bool Handled
        {
            get { return _handled; }
            set { _handled = value; }
        }

        /// <summary>
        /// Set/Get whether the text should be reset the last good value.
        /// </summary>
        public bool ResetText
        {
            get { return _resetText; }
            set { _resetText = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the ValidateTextError event of a control  
    /// </summary>
    public delegate void ValidateTextErrorHandler(object sender, ValidateTextErrorEventArgs e);


}
