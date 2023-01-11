//
//      FILE:   Translator.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
using System;
using System.Windows.Forms;
using System.Resources;
namespace Infralution.Common
{
	/// <summary>
	/// Provides a simple mechanism for translating programmatic strings using .NET resource files.
	/// </summary>
	/// <remarks>
	/// The client code should define a class (or module) with a static property or member declared
	/// of this type.  A resX file with a name matching that of the class or module should be added
	/// to the project to contain the required strings to be translated.  The translator methods can
	/// then be used by classes within the project to translate programmatic strings.
	/// </remarks>
	/// <example>
	/// The following sample demonstrates typical usage of the translator within a project
	/// <code lang ="C#">
    /// internal sealed class Translation
    /// {    
    ///     public static Infralution.Common.Translator Translator = new Infralution.Common.Translator(typeof(Translation)); 
    /// }   
    /// 
    /// public class MyClass
    /// {
    ///     public void DoSomething()
    ///     {
    ///         Translation.Translator.ShowMessage("MyMessageTitle", "MyMessageText");
    ///     }
    /// }
	/// </code>
    /// <code lang ="Visual Basic">
    /// Module Translation
    ///     Public Translator as new Infralution.Common.Translator(GetType(Translation)); 
    /// End Module   
    /// 
    /// Public Class MyClass
    ///
    ///     Public Sub DoSomething()
    ///         Translator.ShowMessage("MyMessageTitle", "MyMessageText");
    ///     End Sub
    ///     
    /// End Class
    /// </code>
    /// A ResX file called Translation.resx would also be added to the project with string resources
    /// called "MyMessageTitle" and "MyMessageText".  Note that Visual Studio may hide ResX files which have
	/// the same name as code files.  In this case use the "Project->Show All Files" menu to display the
	/// files within the solution explorer.
	/// </example>
    [Obsolete("This class is now obsolete.  Use VS2005 Properties/Resources and MessageBoxEx methods instead.")]
	public class Translator
	{
        private ResourceManager _rm;

        /// <summary>
        /// Create a new translator using resources associated with the given type
        /// </summary>
        /// <param name="resourceSource">The type that translation resources are associated with</param>
		public Translator(System.Type resourceSource)
		{
            _rm = new ResourceManager(resourceSource);
		}

        /// <summary>
        /// Translate the given string using the resources associated with this translator
        /// </summary>
        /// <remarks>If no translation is found then the original text is returned</remarks>
        /// <param name="text">The programmatic string to translate</param>
        /// <returns>The translated string</returns>
        public string Translate(string text)
        {
            string result = _rm.GetString(text);
            if (result == null)
            {
                return text;    
            }
            return result;
        }

        /// <summary>
        /// Translate the given format string using the resources associated with this translator
        /// and format the given arguments using the translated string
        /// </summary>
        /// <param name="format">The string to translate and format the arguments with</param>
        /// <param name="arg0">Argument to be formatted</param>
        /// <returns>Formatted, translated string</returns>
        public string Translate(string format, object arg0)
        {
            return string.Format(Translate(format), arg0);
        }

        /// <summary>
        /// Translate the given format string using the resources associated with this translator
        /// and format the given arguments using the translated string
        /// </summary>
        /// <param name="format">The string to translate and format the arguments with</param>
        /// <param name="arg0">Argument to be formatted</param>
        /// <param name="arg1">Argument to be formatted</param>
        /// <returns>Formatted, translated string</returns>
        public string Translate(string format, object arg0, object arg1)
        {
            return string.Format(Translate(format), arg0, arg1);
        }

        /// <summary>
        /// Translate the given format string using the resources associated with this translator
        /// and format the given arguments using the translated string
        /// </summary>
        /// <param name="format">The string to translate and format the arguments with</param>
        /// <param name="arg0">Argument to be formatted</param>
        /// <param name="arg1">Argument to be formatted</param>
        /// <param name="arg2">Argument to be formatted</param>
        /// <returns>Formatted, translated string</returns>
        public string Translate(string format, object arg0, object arg1, object arg2)
        {
            return string.Format(Translate(format), arg0, arg1, arg2);
        }

        /// <summary>
        /// Display an Error Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The MessageBox caption string to translate</param>
        /// <param name="text">The MessageBox text string to translate</param>
        public void ShowError(string caption, string text)
        {
            MessageBox.Show(Translate(text), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        /// <summary>
        /// Display an Error Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        public void ShowError(string caption, string textFormat, object arg0)
        {
            MessageBox.Show(Translate(textFormat, arg0), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        /// <summary>
        /// Display an Error Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        public void ShowError(string caption, string textFormat, object arg0, object arg1)
        {
            MessageBox.Show(Translate(textFormat, arg0, arg1), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Error);
        }


        /// <summary>
        /// Display an Error Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        public void ShowError(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            MessageBox.Show(Translate(textFormat, arg0, arg1, arg2), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        /// <summary>
        /// Display an Warning Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The MessageBox caption string to translate</param>
        /// <param name="text">The MessageBox text string to translate</param>
        public void ShowWarning(string caption, string text)
        {
            MessageBox.Show(Translate(text), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        /// <summary>
        /// Display an Warning Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        public void ShowWarning(string caption, string textFormat, object arg0)
        {
            MessageBox.Show(Translate(textFormat, arg0), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        /// <summary>
        /// Display an Warning Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        public void ShowWarning(string caption, string textFormat, object arg0, object arg1)
        {
            MessageBox.Show(Translate(textFormat, arg0, arg1), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        /// <summary>
        /// Display an Warning Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        public void ShowWarning(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            MessageBox.Show(Translate(textFormat, arg0, arg1, arg2), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        /// <summary>
        /// Display an Information Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The MessageBox caption string to translate</param>
        /// <param name="text">The MessageBox text string to translate</param>
        public void ShowMessage(string caption, string text)
        {
            MessageBox.Show(Translate(text), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// Display an Information Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        public void ShowMessage(string caption, string textFormat, object arg0)
        {
            MessageBox.Show(Translate(textFormat, arg0), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// Display an Information Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        public void ShowMessage(string caption, string textFormat, object arg0, object arg1)
        {
            MessageBox.Show(Translate(textFormat, arg0, arg1), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// Display an Information Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        public void ShowMessage(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            MessageBox.Show(Translate(textFormat, arg0, arg1, arg2), Translate(caption), MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// Display an Question Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The MessageBox caption string to translate</param>
        /// <param name="text">The MessageBox text string to translate</param>
        public DialogResult ShowQuestion(string caption, string text)
        {
            return MessageBox.Show(Translate(text), Translate(caption), MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

        /// <summary>
        /// Display an Question Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        public DialogResult ShowQuestion(string caption, string textFormat, object arg0)
        {
            return MessageBox.Show(Translate(textFormat, arg0), Translate(caption), MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

        /// <summary>
        /// Display an Question Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        public DialogResult ShowQuestion(string caption, string textFormat, object arg0, object arg1)
        {
            return MessageBox.Show(Translate(textFormat, arg0, arg1), Translate(caption), MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

        /// <summary>
        /// Display an Question Message Box using translations of the given text and caption.
        /// </summary>
        /// <param name="caption">The caption to translate for the MessageBox caption</param>
        /// <param name="textFormat">The Message Box text string to translate, format arguments with</param>
        /// <param name="arg0">Argument to format as the MessageBox text</param>
        /// <param name="arg1">Argument to format as the MessageBox text</param>
        /// <param name="arg2">Argument to format as the MessageBox text</param>
        public DialogResult ShowQuestion(string caption, string textFormat, object arg0, object arg1, object arg2)
        {
            return MessageBox.Show(Translate(textFormat, arg0, arg1, arg2), Translate(caption), MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

    }
}
