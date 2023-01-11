using System;
using System.Windows.Forms;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Extension methods for safely invoking.
    /// </summary>
    public static class InvokeExtensions
    {
        /// <summary>
        /// Safely invoke an EventHandler<typeparam name="T">T</typeparam>.
        /// </summary>
        public static void SafeInvoke<T>(this EventHandler<T> handler, object sender, T args) where T : EventArgs
        {
            // thread-safe by implicit copy onto the stack (Eventhandler is immutable) (don't need var copy = handler)
            if (handler != null)
            {
                handler(sender, args);
            }
        }

        /// <summary>
        /// Safely invoke an EventHandler.
        /// </summary>
        public static void SafeInvoke(this EventHandler handler, object sender)
        {
            // thread-safe by implicit copy onto the stack (don't need var copy = handler)
            if (handler != null)
            {
                handler(sender, EventArgs.Empty);
            }
        }

        /// <summary>
        /// Invokes the given action on the given control's UI thread, if invocation is needed.
        /// </summary>
        /// <param name="control">Control on whose UI thread to possibly invoke.</param>
        /// <param name="action">Action to be invoked on the given control.</param>
        public static void InvokeIfRequired(this Control control, Action action)
        {
            if (control != null && control.InvokeRequired)
            {
                control.Invoke(action);
            }
            else
            {
                action();
            }
        }

        /// <summary>
        /// Invokes the given action on the given control's UI thread, if invocation is needed.
        /// </summary>
        /// <param name="control">Control on whose UI thread to possibly invoke.</param>
        /// <param name="action">Action to be invoked on the given control.</param>
        public static T InvokeIfRequired<T>(this Control control, Func<T> action)
        {
            if (control != null && control.InvokeRequired)
            {
                T result = default(T);
                control.Invoke((MethodInvoker)(() => { result = action(); }));
                return result;
            }
            else
            {
                return action();
            }
        }

        /// <summary>
        /// Begin invokes the given action on the given control's UI thread, if invocation is needed.
        /// </summary>
        /// <param name="control">Control on whose UI thread to possibly invoke.</param>
        /// <param name="action">Action to be invoked on the given control.</param>
        public static void BeginInvokeIfRequired(this Control control, Action action)
        {
            if (control != null && control.InvokeRequired)
            {
                control.BeginInvoke(action);
            }
            else
            {
                action();
            }
        }
    }
}
