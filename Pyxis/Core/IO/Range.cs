namespace Pyxis.Core.IO
{
    /// <summary>
    /// Represent a range of values
    /// </summary>
    /// <typeparam name="T">value domain</typeparam>
    public class Range<T> 
    {
        /// <summary>
        /// Minimum boundary of the range
        /// </summary>        
        public T Min { get; set; }

        /// <summary>
        /// Maximum boundary of the range
        /// </summary>
        public T Max { get; set; }
    }
}
