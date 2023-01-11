#region File Header
//
//      FILE:   DropDownButton.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion

using System.ComponentModel;
using System.Windows.Forms.VisualStyles;

namespace Infralution.Controls
{
	/// <summary>
	/// Defines a drop down button that uses Visual Styles  
	/// </summary>
	/// <remarks>
	/// Designed for use in combo boxes and dropdown lists.
	/// </remarks>
    [ToolboxItem(false)]
    public class DropDownButton : ThemedButton
	{
        #region Public Interface

        /// <summary>
        /// Hides the Text property in the designer
        /// </summary>
        [EditorBrowsable(EditorBrowsableState.Never),
        Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public override string Text
        {
            get { return base.Text; }
            set { base.Text = null; }
        }

        #endregion

        #region Local Methods

        protected override VisualStyleElement GetVisualStyleElement(VisualState visualState)
        {
            switch (visualState)
            {
                case VisualState.Normal:
                    return VisualStyleElement.ComboBox.DropDownButton.Normal;
                case VisualState.Disabled:
                    return VisualStyleElement.ComboBox.DropDownButton.Disabled;
                case VisualState.Hot:
                    return VisualStyleElement.ComboBox.DropDownButton.Hot;
                case VisualState.Pressed:
                    return VisualStyleElement.ComboBox.DropDownButton.Pressed;
            }
            return VisualStyleElement.ComboBox.DropDownButton.Normal;
        }

        ///// <summary>
        ///// Return the default image to use when XP Themes are not being used
        ///// </summary>
        //private Image DefaultImage
        //{
        //    get
        //    {
        //        if (_defaultImage == null)
        //        {
        //            Icon icon = new Icon(typeof(DropDownButton), "Icons.DropDownEdit.ico");
        //            _defaultImage = Bitmap.FromHicon(icon.Handle);
        //        }
        //        return _defaultImage;
        //    }
        //}


        #endregion
	}
}
