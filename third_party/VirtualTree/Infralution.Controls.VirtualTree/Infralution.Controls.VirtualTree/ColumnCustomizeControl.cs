#region File Header
//
//      FILE:   ColumnCustomizeControl.cs.
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
using System.Windows.Forms;
using System.Collections;
using System.Drawing;
using System.ComponentModel;
namespace Infralution.Controls.VirtualTree
{
	/// <summary>
	/// Defines the control used to display non-active <see cref="Column">Columns</see> when the user
	/// customizes a <see cref="VirtualTree"/>.
	/// </summary>
	/// <remarks>
	/// This control can be used or extended by applications to provide specialized handling of customization.
	/// </remarks>
    /// <seealso cref = "VirtualTree"/>
    /// <seealso cref = "Column"/>
    /// <seealso cref = "ColumnCustomizeForm"/>
    public class ColumnCustomizeControl : WidgetControl
    {

        #region Member Variables

        /// <summary>
        /// The tree that this control is customizing the columns of
        /// </summary>
        private VirtualTree _tree;

        /// <summary>
        /// Scrollbar for scrolling
        /// </summary>
        private System.Windows.Forms.VScrollBar _vertScroll;

        /// <summary>
        /// Required by Forms Designers
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// The widget that contains the column header widgets
        /// </summary>
        private ColumnCustomizeWidget _customizeWidget;

        #endregion
 
        #region Component Designer generated code

        private void InitializeComponent()
        {
            this._vertScroll = new System.Windows.Forms.VScrollBar();
            this.SuspendLayout();
            // 
            // _vertScroll
            // 
            this._vertScroll.Location = new System.Drawing.Point(17, 17);
            this._vertScroll.Name = "_vertScroll";
            this._vertScroll.TabIndex = 0;
            this._vertScroll.Visible = false;
            this._vertScroll.ValueChanged += new System.EventHandler(this.OnVerticalScroll);
            // 
            // ColumnCustomizeControl
            // 
            this.AllowDrop = true;
            this.Controls.Add(this._vertScroll);
            this.ResumeLayout(false);

        }

        /// <summary>
        /// Dispose of this control and any resources 
        /// </summary>
        /// <param name="disposing"></param>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if( components != null )
                    components.Dispose();
                Tree = null;
            }
            base.Dispose (disposing);
        }

        #endregion

        #region Public Interface

        /// <summary>
        /// Default constructor
        /// </summary>
        public ColumnCustomizeControl()
        {
            InitializeComponent();
            _customizeWidget = new ColumnCustomizeWidget(this);
            this.Widgets.Add(_customizeWidget);
        }
        
        /// <summary>
        /// Get/Set the Virtual Tree control that is to have its columns customized
        /// </summary>
        [Category("Data"),
         DefaultValue(null),
         Description("The Virtual Tree control that is to have its columns customized")] 
        public VirtualTree Tree
        {
            get { return _tree; }
            set 
            { 
                if (_tree != null)
                {
                    _tree.Columns.ListChanged -= new ListChangedEventHandler(OnColumnsChanged);
                }
                _tree = value; 
                if (_tree != null)
                {
                    _tree.Columns.ListChanged += new ListChangedEventHandler(OnColumnsChanged);
                }
                PerformLayout();
            }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Return the widget used to layout the column header widgets
        /// </summary>
        protected ColumnCustomizeWidget ColumnCustomizeWidget
        {
            get { return _customizeWidget; }
        }

        /// <summary>
        /// Get the vertical scrollbar used by the control
        /// </summary>
        protected VScrollBar VertScrollBar
        {
            get { return _vertScroll; }
        }

        /// <summary>
        /// Handles changes to the control layout
        /// </summary>
        /// <param name="levent"></param>
        protected override void OnLayout(LayoutEventArgs levent)
        {
            if (Tree != null)
            {
                this.RightToLeft = Tree.RightToLeft;
            }
            Rectangle bounds = new Rectangle(ClientSize.Width - VertScrollBar.Width, 0, 
                VertScrollBar.Width, ClientSize.Height);
            VertScrollBar.Bounds = RtlTranslateRect(bounds);
            base.OnLayout (levent);
        }


        /// <summary>
        /// Update the display widgets following a change to the tree columns 
        /// </summary>
        protected override void LayoutWidgets()
        {
            int requiredHeight = _customizeWidget.RequiredHeight;

            bool showScroll = requiredHeight > ClientSize.Height;
            if (showScroll)
            {
                VertScrollBar.LargeChange = ClientSize.Height;
                VertScrollBar.Maximum = requiredHeight;
                int maxValue = requiredHeight - ClientSize.Height;
                if (VertScrollBar.Value > maxValue)
                    VertScrollBar.Value = maxValue;
            }
            else
            {
                VertScrollBar.Value = 0;
            }

            int width = (VertScrollBar.Visible) ? ClientSize.Width - VertScrollBar.Width : ClientSize.Width;
            int height = Math.Max(ClientSize.Height, requiredHeight);
            Rectangle bounds = new Rectangle(0, -VertScrollBar.Value, width, height);

            if (showScroll != VertScrollBar.Visible)
            {
                bounds.Width = (showScroll) ? ClientSize.Width - VertScrollBar.Width : ClientSize.Width;
            }

            _customizeWidget.Bounds = RtlTranslateRect(bounds);

            VertScrollBar.Visible = showScroll;
        }

        /// <summary>
        /// Handle changes to the displayed columns
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnColumnsChanged(object sender, ListChangedEventArgs e)
        {
            PerformLayout();
        }

        /// <summary>
        /// Handles dropping columns on the control to make them inactive
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragEnter(DragEventArgs e)
        {
            e.Effect = DragDropEffects.None;
            if (e.Data.GetDataPresent(typeof(Column)))
            {
                Column dropColumn = (Column)e.Data.GetData(typeof(Column));
                if (dropColumn.Active && dropColumn.Tree == Tree && dropColumn.Hidable)
                {
                    e.Effect = DragDropEffects.Move;
                }
            }
        }

        /// <summary>
        /// Handles dropping columns on the control to make them inactive
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragOver(DragEventArgs e)
        {
        }

        /// <summary>
        /// Overridden to handle dropping columns on the control to make them inactive
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragLeave(EventArgs e)
        {
        }

        /// <summary>
        /// Handle dropping columns on the control to make them inactive
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragDrop(DragEventArgs e)
        {
            if (e.Data.GetDataPresent(typeof(Column)))
            {
                Column dropColumn = (Column)e.Data.GetData(typeof(Column));
                if (dropColumn.Tree == Tree)
                {
                    dropColumn.Active = false;
                }
            }
        }

        /// <summary>
        /// Handle a vertical scroll event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnVerticalScroll(object sender, System.EventArgs e)
        {
            PerformLayout();        
        }


        #endregion

     }
}
