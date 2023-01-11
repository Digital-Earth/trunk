using Pyxis.Contract.Publishing;

namespace PyxisCLI.Server.Models
{
    /// <summary>
    /// repsonse model for styles to include style hash to be used
    /// </summary>
    public class StyleAndHash
    {
        /// <summary>
        /// Style
        /// </summary>
        public Style Style { get; set; }

        /// <summary>
        /// If present, this hash can be used to active this style
        /// </summary>
        public string Hash { get; set; }
    }
}
