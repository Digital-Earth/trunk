using System;
using System.ComponentModel;
using System.Windows.Forms;
using System.Windows.Forms.Design;

namespace Infralution.Controls
{
	/// <summary>
	/// Defines an enhanced PropertyGrid control which supports a context menu to reset items and
	/// supports display of an Events Tab.
	/// </summary>
	public class EnhancedPropertyGrid : PropertyGrid
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private Container components = null;
        private MenuItem _resetItem;
        private bool _showEventsTab = false;

        /// <summary>
        /// Default constructor
        /// </summary>
		public EnhancedPropertyGrid()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

            // Initialize the context menu
            _resetItem = new MenuItem("&Reset", new EventHandler(OnResetProperty));
            ContextMenu = new ContextMenu();
            ContextMenu.MenuItems.Add(_resetItem);
            ContextMenu.Popup += new EventHandler(OnContextMenuPopup);
		}

        /// <summary>
        /// Show or Hide the events tab 
        /// </summary>
        /// <param name="value">Should the tab be shown or hidden</param>
        /// <param name="site">The component site hosting the events to be displayed in the tab</param>
        public void ShowEventsTab(bool value, ISite site)
        {
            this.Site = site;
            if (value != _showEventsTab)
            {
                if (value)
                    PropertyTabs.AddTabType(typeof(EventsTab));
                else
                    PropertyTabs.RemoveTabType(typeof(EventsTab));                    
                ShowEventsButton(value);
                _showEventsTab = value;
            }
        }

        /// <summary>
        /// Handle resetting the currently selected property
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnResetProperty(object sender, EventArgs e)
        {
            GridItem selectedItem = this.SelectedGridItem;
            object oldValue = selectedItem.Value;
            this.ResetSelectedProperty();
            this.OnPropertyValueChanged(new PropertyValueChangedEventArgs(selectedItem, oldValue));
        }

        /// <summary>
        /// Return true if the given grid item can be reset
        /// </summary>
        /// <param name="item">The item to check</param>
        /// <returns>True if the item can be reset</returns>
        protected virtual bool CanReset(GridItem item)
        {
            bool canReset = false;
            if (item != null && item.PropertyDescriptor != null)
            {
                object component = item.Parent.Value;
                if (component == null)
                {
                    if (SelectedObjects != null && SelectedObjects.Length > 1)
                        component = SelectedObjects;
                    else
                        component = SelectedObject;
                }
                canReset = item.PropertyDescriptor.CanResetValue(component);
                if (!canReset)
                {
                    foreach (GridItem subItem in item.GridItems)
                    {
                        if (CanReset(subItem))
                        {
                            canReset = true;
                            break;
                        }
                    }
                }
            }
            return canReset;
       }

        /// <summary>
        /// Enabled/Disable context menu items
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnContextMenuPopup(object sender, EventArgs e)
        {
            _resetItem.Enabled = CanReset(SelectedGridItem);
        }
 
		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
        }

        #endregion


    }
}
