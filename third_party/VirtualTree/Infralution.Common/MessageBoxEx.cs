//
//      FILE:   MessageBoxEx.cs.
//
// COPYRIGHT:   Copyright 2007 
//
using System;
using System.Windows.Forms;
using System.Resources;
namespace Infralution.Common
{
	/// <summary>
	/// Provides utility methods for showing common message dialogs.
	/// </summary>
	public static class MessageBoxEx
	{

        /// <summary>
        /// Display an Error Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="text">The MessageBox text</param>
        static public void ShowError(string caption, string text)
        {
            MessageBox.Show(text, caption, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        /// <summary>
        /// Display an Error Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        static public void ShowError(string caption, string textFormat, object arg0)
        {
            ShowError(caption, string.Format(textFormat, arg0));
        }

        /// <summary>
        /// Display an Error Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        static public void ShowError(string caption, string textFormat, object arg0, object arg1)
        {
            ShowError(caption, string.Format(textFormat, arg0, arg1));
        }

        /// <summary>
        /// Display an Error Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        static public void ShowError(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            ShowError(caption, string.Format(textFormat, arg0, arg1, arg2));
        }

        /// <summary>
        /// Display a Warning Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="text">The MessageBox text</param>
        static public void ShowWarning(string caption, string text)
        {
            MessageBox.Show(text, caption, MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        /// <summary>
        /// Display a Warning Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        static public void ShowWarning(string caption, string textFormat, object arg0)
        {
            ShowWarning(caption, string.Format(textFormat, arg0));
        }

        /// <summary>
        /// Display a Warning Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        static public void ShowWarning(string caption, string textFormat, object arg0, object arg1)
        {
            ShowWarning(caption, string.Format(textFormat, arg0, arg1));
        }

        /// <summary>
        /// Display a Warning Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        static public void ShowWarning(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            ShowWarning(caption, string.Format(textFormat, arg0, arg1, arg2));
        }

        /// <summary>
        /// Display an Information Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="text">The MessageBox text</param>
        static public void ShowMessage(string caption, string text)
        {
            MessageBox.Show(text, caption, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// Display an Information Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        static public void ShowMessage(string caption, string textFormat, object arg0)
        {
            ShowMessage(caption, string.Format(textFormat, arg0));
        }

        /// <summary>
        /// Display an Information Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        static public void ShowMessage(string caption, string textFormat, object arg0, object arg1)
        {
            ShowMessage(caption, string.Format(textFormat, arg0, arg1));
        }

        /// <summary>
        /// Display an Information Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        static public void ShowMessage(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            ShowMessage(caption, string.Format(textFormat, arg0, arg1, arg2));
        }

        /// <summary>
        /// Display a Question Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="text">The MessageBox text</param>
        static public DialogResult ShowQuestion(string caption, string text)
        {
            return MessageBox.Show(text, caption, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

        /// <summary>
        /// Display a Question Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        static public DialogResult ShowQuestion(string caption, string textFormat, object arg0)
        {
            return ShowQuestion(caption, string.Format(textFormat, arg0));
        }

        /// <summary>
        /// Display a Question Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        static public DialogResult ShowQuestion(string caption, string textFormat, object arg0, object arg1)
        {
            return ShowQuestion(caption, string.Format(textFormat, arg0, arg1));
        }

        /// <summary>
        /// Display a Question Message Box.
        /// </summary>
        /// <param name="caption">The MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        static public DialogResult ShowQuestion(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            return ShowQuestion(caption, string.Format(textFormat, arg0, arg1, arg2));
        }

    }
}
