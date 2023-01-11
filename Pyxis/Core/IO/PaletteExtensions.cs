using System;
using System.Collections.Generic;
using Pyxis.Contract.Converters;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.IO
{
    /// <summary>
    /// Provides a variety of palette extension services.
    /// </summary>
    public static class PaletteExtensions
    {
        /// <summary>
        /// Get the default Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        public static Palette Default
        {
            get
            {
                return Palette.Create(
                    CssColorConverter.FromCss("#330083"),
                    CssColorConverter.FromCss("#9b0093"),
                    CssColorConverter.FromCss("#ec0929"),
                    CssColorConverter.FromCss("#f35633"),
                    CssColorConverter.FromCss("#fffd4d")
                );
            }
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.Palette from a PYXIS SDK palette.
        /// </summary>
        /// <param name="from">The PYXIS SDK palette to use.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.Palette.</returns>
        public static Palette ToContractPalette(this ApplicationUtility.Visualization.Palette from)
        {
            var palette = new Palette() { Steps = new List<Palette.Step>() };

            if (from.IsStringPalette)
            {
                var stringPalette = from.GetStringPalette();

                foreach (var step in stringPalette.Steps)
                {
                    palette.Steps.Add(new Palette.Step() { Value = step.Key, Color = step.Value });
                }
            }
            else
            {
                var numericPalette = from.GetNumericPalette();

                foreach (var step in numericPalette.Steps)
                {
                    palette.Steps.Add(new Palette.Step() { Value = step.Value, Color = step.Color });
                }
            }

            return palette;
        }

        /// <summary>
        /// Get the palette as a StringPalette.
        /// </summary>
        /// <param name="palette">The palette</param>
        /// <returns>The string palette</returns>
        public static ApplicationUtility.Visualization.StringPalette AsStringPalette(this Palette palette)
        {
            var stringPalette = new ApplicationUtility.Visualization.StringPalette();

            foreach (var step in palette.Steps)
            {
                stringPalette.Steps[step.Value as string] = step.Color;
            }

            return stringPalette;
        }

        /// <summary>
        /// Get the palette as a NumericPalette.
        /// </summary>
        /// <param name="palette">The palette</param>
        /// <returns>The numeric palette</returns>
        public static ApplicationUtility.Visualization.NumericPalette AsNumericPalette(this Palette palette)
        {
            var numericPalette = new ApplicationUtility.Visualization.NumericPalette();

            foreach (var step in palette.Steps)
            {
                numericPalette.Steps.Add(new ApplicationUtility.Visualization.NumericPalette.Step()
                {
                    Value = Convert.ToDouble(step.Value),
                    Color = step.Color
                });
            }

            return numericPalette;
        }

        /// <summary>
        /// Generate a string representation of the Pyxis.UI.Layers.Globe.Palette that can be used by the PYXIS SDK.
        /// </summary>
        /// <returns>The generated string.</returns>
        public static string ToPyxisString(this Palette palette)
        {
            if (palette.Steps[0].Value is string)
            {
                return ToPyxisStringAsStringPalette(palette);
            }
            else
            {
                return ToPyxisStringAsNumericPalette(palette);
            }
        }

        /// <summary>
        /// Generate a string representation of the Pyxis.UI.Layers.Globe.Palette that can be used by the PYXIS SDK.
        /// Force conversion to a string palette. For general conversion use ToPyxisString().
        /// </summary>        
        /// <returns>The generated string.</returns>
        public static string ToPyxisStringAsStringPalette(this Palette palette)
        {
            return new ApplicationUtility.Visualization.Palette(palette.AsStringPalette()).ToString();
        }

        /// <summary>
        /// Generate a string representation of the Pyxis.UI.Layers.Globe.Palette that can be used by the PYXIS SDK.
        /// Force conversion to numeric values palette. For general conversion use ToPyxisString().
        /// </summary>        
        /// <returns>The generated string.</returns>
        public static string ToPyxisStringAsNumericPalette(this Palette palette)
        {
            return new ApplicationUtility.Visualization.Palette(palette.AsNumericPalette()).ToString();   
        }
    }
}