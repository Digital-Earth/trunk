using System;
using System.Threading.Tasks;

namespace Pyxis.UI
{
    /// <summary>
    /// Utility class to perform animation for a specific duration
    /// 
    /// This class accept a callback function that get a single double argument - the progress
    /// the progress will start from 0 and will get to 1 when animation is completed.
    /// 
    /// Once the animation is completed the animation object will be deleted and will stop invoking the callback.
    /// While the animation is in progress, the callback will be called on every frame.
    /// 
    /// You can use <see cref="Then"/> function to perform action once the animation completed:
    /// 
    /// <example>
    /// PyxisView
    ///    .Animate((progress)=> { Layer.Opacity = 1-progress; },TimeSpan.FromSeconds(0.5))
    ///    .Then(()=>{ PyxisView.RemoveLayer(layer) });
    /// </example>
    /// 
    /// </summary>
    /// <seealso cref="PyxisView.Animate"/>
    public class Animation
    {
        private readonly PyxisView m_view;
        private readonly Action<double> m_animationCallback;
        private readonly TimeSpan m_duration;
        private readonly DateTime m_start;
        private readonly TaskCompletionSource<object> m_animationTaskSource;

        /// <summary>
        /// The task that can be checked for animation completion.
        /// </summary>
        public Task AnimationCompleted
        {
            get { return m_animationTaskSource.Task; }
        }

        internal Animation(PyxisView view, Action<double> animationCallback, TimeSpan duration)
        {
            m_view = view;
            m_animationCallback = animationCallback;
            m_duration = duration;
            m_start = DateTime.Now;

            m_animationTaskSource = new TaskCompletionSource<object>();

            m_view.InvokeBeforePaintStarted(Animate);
        }

        private void Animate()
        {
            var now = DateTime.Now;

            //if animation in progress
            if (m_duration.TotalMilliseconds > 0 &&
                now - m_start < m_duration)
            {
                //progress animation 
                m_animationCallback((now - m_start).TotalMilliseconds/m_duration.TotalMilliseconds);

                //schedule next frame
                m_view.InvokeBeforePaintStarted(Animate);
            }
            else
            {
                //mark as completed
                m_animationCallback(1.0);

                //complete the task
                m_animationTaskSource.SetResult(null);
            }
        }

        /// <summary>
        /// Perform an action after the animation has completed.
        /// </summary>
        /// <param name="action">The action.</param>
        public void Then(Action action)
        {
            AnimationCompleted.ContinueWith(task =>
            {
                action();
            }, TaskScheduler.FromCurrentSynchronizationContext());
        }
    }
}
