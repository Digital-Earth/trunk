/******************************************************************************
ProgressData.cs

begin      : March 9, 2010
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/


namespace Pyxis.Utilities
{

    /// <summary>
    /// Holds a value (CompletedAmount) that is updated as progress is made.
    /// </summary>
    public class ProgressData
    {
        /// <summary>
        /// Gets or sets the title or the name of the thing being measured.
        /// </summary>
        /// <value>The title.</value>
        public string Title { get; set;}

        /// <summary>
        /// Gets or sets the units that this element is measured in.  Typically 
        /// this will be set to "bytes", "files", "tiles", "seconds", etc.
        /// </summary>
        /// <value>The units.</value>
        public string Units { get; set;}

        /// <summary>
        /// Gets or sets the completed amount, which is a value between 0 and 1 
        /// that indicates what proportion of the work has been completed.
        /// </summary>
        /// <value>The completed amount.</value>
        public ObservableObject<double> CompletedAmount { get; private set;}

        private ThreadSafeInt m_FinalValue = 1;
        /// <summary>
        /// Gets or sets the final value.
        /// </summary>
        /// <value>The final value.</value>
        public int FinalValue
        {
            get
            {
                return m_FinalValue;
            }
            set
            {
                m_FinalValue = value;
                UpdateProgress();
            }
        }

        /// <summary>
        /// Increments the final value.
        /// </summary>
        /// <param name="increment">The increment.</param>
        public void IncrementFinalValue(int increment)
        {
            FinalValue += increment;
            UpdateProgress();
        }

        private ObservableObject<int> m_CurrentValue = new ObservableObject<int>(0);
        /// <summary>
        /// Gets or sets the current value.
        /// </summary>
        /// <value>The current value.</value>
        public ObservableObject<int> CurrentValue
        {
            get
            {
                return m_CurrentValue;
            }
            private set
            {
                m_CurrentValue = value;
                UpdateProgress();
            }
        }

        private object m_currentValueLock = new object();

        /// <summary>
        /// Increments the current value.
        /// </summary>
        /// <param name="increment">The increment.</param>
        public void IncrementCurrentValue(int increment)
        {
            lock (m_currentValueLock)
            {
                CurrentValue.Value += increment;
            }
            UpdateProgress();
        }

        /// <summary>
        /// Updates the progress.
        /// </summary>
        private void UpdateProgress()
        {
            if (FinalValue > 0)
            {
                CompletedAmount.Value = ((double) CurrentValue.Value) / FinalValue;
            }
            else
            {
                CompletedAmount.Value = 0;
            }
        }

        public ProgressData()
        {
            CompletedAmount = new ObservableObject<double>(0.0);

            CurrentValue = new ObservableObject<int>(0);
            CurrentValue.Changed += (o, args) => UpdateProgress();            
        }

        public override string ToString()
        {
            return string.Format("{0} {1} of {2} {3} ({4:00.00}%)",
                Title, CurrentValue.Value, FinalValue, Units, CompletedAmount.Value * 100);
        }
    }
}
