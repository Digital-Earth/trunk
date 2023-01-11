/******************************************************************************
ImageComboBox.cs

begin      : August 17, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
reference  : Downloaded from http://www.csharphelp.com/archives/archive280.html.
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Extends the standard ComboBox to support display of images.
    /// </summary>
    public class ImageComboBox : ComboBox
    {
        public ImageList ImageList
        {
            get
            {
                return m_imageList;
            }
        }
        private ImageList m_imageList;

        /// <summary>
        /// Initializes a new instance of the <see cref="ImageComboBox"/> class.
        /// </summary>
        /// <param name="imageList">The image list.</param>
        public ImageComboBox(ImageList imageList)
        {
            m_imageList = imageList;
            DrawMode = DrawMode.OwnerDrawFixed;
        }

        protected override void OnDrawItem(DrawItemEventArgs ea)
        {
            ea.DrawBackground();
            ea.DrawFocusRectangle();

            Item item;
            Size imageSize = m_imageList.ImageSize;
            Rectangle bounds = ea.Bounds;

            try
            {
                item = (Item)Items[ea.Index];

                if (item.ImageIndex != -1)
                {
                    m_imageList.Draw(ea.Graphics, bounds.Left, bounds.Top,
                        item.ImageIndex);
                    ea.Graphics.DrawString(item.Text, ea.Font, new
                        SolidBrush(ea.ForeColor), bounds.Left + imageSize.Width, bounds.Top);
                }
                else
                {
                    ea.Graphics.DrawString(item.Text, ea.Font, new
                        SolidBrush(ea.ForeColor), bounds.Left, bounds.Top);
                }
            }
            catch
            {
                if (ea.Index != -1)
                {
                    ea.Graphics.DrawString(Items[ea.Index].ToString(), ea.Font, new
                        SolidBrush(ea.ForeColor), bounds.Left, bounds.Top);
                }
                else
                {
                    ea.Graphics.DrawString(Text, ea.Font, new
                        SolidBrush(ea.ForeColor), bounds.Left, bounds.Top);
                }
            }

            base.OnDrawItem(ea);
        }

        /// <summary>
        /// The type used to represent each item in an ImageComboBox.
        /// </summary>
        public class Item
        {
            public string Text
            {
                get { return m_text; }
                set { m_text = value; }
            }
            private string m_text;

            public int ImageIndex
            {
                get { return m_imageIndex; }
                set { m_imageIndex = value; }
            }
            private int m_imageIndex;

            public Item()
                : this("")
            {
            }

            public Item(string text)
                : this(text, -1)
            {
            }

            public Item(string text, int imageIndex)
            {
                m_text = text;
                m_imageIndex = imageIndex;
            }

            public override string ToString()
            {
                return m_text;
            }
        }
    }
}
