using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Contract.Converters
{
    /// <summary>
    /// Utility to convert between cascading style sheets color specifiers and System.Drawing.Color representations of color.
    /// </summary>
    public static class CssColorConverter
    {
        /// <summary>
        /// Convert from a cascading style sheets color to a System.Drawing.Color.
        /// </summary>
        /// <param name="colorString">Cascading style sheets color string to convert.</param>
        /// <returns>System.Drawing.Color corresponding to <paramref name="colorString"/>.</returns>
        public static Color FromCss(string colorString)
        {
            if (colorString == null) return Color.Empty;
            colorString = colorString.ToLower().Trim();            
            
            if (colorString.StartsWith("rgb("))
            {
                var rgb = colorString.Remove(0, 4).TrimEnd(')').Split(',').Select(x => int.Parse(x)).ToList();
                return Color.FromArgb(rgb[0], rgb[1], rgb[2]);
            }
            if (colorString.StartsWith("rgba("))
            {
                var rgba = colorString.Remove(0, 5).TrimEnd(')').Split(',').ToList();
                var rgb = rgba.Take(3).Select(x => int.Parse(x)).ToList();
                int alpha = 255;
                if (rgba[3].EndsWith("%"))
                {
                    alpha = (int)(255 * double.Parse(rgba[3].Trim('%')) / 100);
                }
                else
                {
                    alpha = (int)(255 * double.Parse(rgba[3]));
                }
                return Color.FromArgb(alpha, rgb[0], rgb[1], rgb[2]);
            }
            return ColorTranslator.FromHtml(colorString);
        }

        /// <summary>
        /// Convert from a System.Drawing.Color to a cascading style sheets color.
        /// </summary>
        /// <param name="color">System.Drawing.Color to convert.</param>
        /// <returns>Cascading style sheets color string corresponding to <paramref name="color"/>.</returns>
        public static string ToCss(Color color)
        {
            if (color.A == 255)
            {
                return ColorTranslator.ToHtml((Color)color);
            }
            else
            {
                return String.Format("rgba({0},{1},{2},{3:0.00})", color.R, color.G, color.B, color.A / 255.0);
            }
        }
    }

    /// <summary>
    /// JSON converter for serializing and deserializing System.Drawing.Color.
    /// </summary>
    public class ColorJsonConverter : JsonConverter
    {
        /// <summary>
        /// Determines if the specified System.Type can be converted.
        /// </summary>
        /// <param name="objectType">The System.Type to determine if it can be converted.</param>
        /// <returns>true if <paramref name="objectType"/> can be converted; otherwise, false.</returns>
        public override bool CanConvert(Type objectType)
        {
            return typeof(Color) == objectType;
        }
        
        /// <summary>
        /// Reads the JSON representation of the object.
        /// </summary>
        /// <param name="reader">The JSON Reader to read from.</param>
        /// <param name="objectType">The System.Type of the object.</param>
        /// <param name="existingValue">The existing value of object being read.</param>
        /// <param name="serializer">The calling serializer.</param>
        /// <returns>The object value.</returns>
        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var colorString = serializer.Deserialize<string>(reader);
            return CssColorConverter.FromCss(colorString);
        }
        
        /// <summary>
        /// Writes the JSON representation of the object.
        /// </summary>
        /// <param name="writer">The JSON writer to write to.</param>
        /// <param name="value">The System.Object.</param>
        /// <param name="serializer">The calling serializer.</param>
        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var color = (Color)value;
            serializer.Serialize(writer, CssColorConverter.ToCss(color));
        }
    }
}
