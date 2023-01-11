#region File Header
//
//      FILE:   TreeDesigner.cs.
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
using System.Collections;
using System.Drawing.Design;
using System.Windows.Forms.Design;
using System.Windows.Forms;
using System.ComponentModel.Design;
using System.ComponentModel;
using System.Reflection;
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Defines the designer for a <see cref="VirtualTree"/>.
    /// </summary>
    internal class TreeDesigner : ControlDesigner, IServiceProvider  
    {

        /// <summary>
        /// Initialise the Virtual Tree Designer
        /// </summary>
        public TreeDesigner()
        {
            Verbs.Add(new DesignerVerb("Edit Virtual Tree", new EventHandler(this.OnEditTree)));
            Verbs.Add(new DesignerVerb("Add Custom Header Menu", new EventHandler(this.OnCustomizeHeaderMenu)));
        }

        /// <summary>
        /// Return the tree the designer is associated with
        /// </summary>
        protected VirtualTree Tree
        {
            get { return (Component as VirtualTree); }
        }

        /// <summary>
        /// Handle the user click on the EditTree
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void OnEditTree(object sender, EventArgs e)
        {
            TreeEditorForm form = new TreeEditorForm(Tree, this);   
            form.ShowDialog();
        }

        /// <summary>
        /// Handle the user click on the CustomizeHeaderMenu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void OnCustomizeHeaderMenu(object sender, EventArgs e)
        {
           Tree.HeaderContextMenu = Tree.CreateHeaderContextMenu(true);   
        }

        /// <summary>
        /// Hook up to the ComponentChangeService to remove associated components
        /// when the VirtualTree is removed
        /// </summary>
        /// <param name="component"></param>
        public override void Initialize(IComponent component)
        {
            base.Initialize (component);

            // hook up the change service to listen for removal of the component
            //
            IComponentChangeService changeService = (IComponentChangeService)this.GetService(typeof(IComponentChangeService));
            if (changeService != null)
            {
                changeService.ComponentRemoving += new ComponentEventHandler(OnComponentRemoving);
            }
        }

        /// <summary>
        /// Return the components associated with the control
        /// </summary>
        public override ICollection AssociatedComponents
        {
            get
            {
                ArrayList list = new ArrayList();
                list.AddRange(Tree.Columns);
                list.AddRange(Tree.RowBindings);
                list.AddRange(Tree.Editors);
                foreach (CellEditor editor in Tree.Editors)
                {
                    list.Add(editor.Control);
                }
                return list;
            }
        }

        /// <summary>
        /// Handle removal of associated components when virtual tree is deleted by the user
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnComponentRemoving(object sender, ComponentEventArgs e)
        {
            if (e.Component == Component)
            {
                IDesignerHost host = (IDesignerHost)GetService(typeof(IDesignerHost));
                foreach (IComponent component in AssociatedComponents)
                {
                    host.DestroyComponent(component);
                }
            }
        }

        #region IServiceProvider Members

        /// <summary>
        /// Implements the IServiceProvide.GetService method
        /// </summary>
        /// <param name="serviceType">The service to get</param>
        /// <returns>The service</returns>
        public new object GetService(Type serviceType)
        {
            return base.GetService(serviceType);
        }

        #endregion

    }

}


