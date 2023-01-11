using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Converters;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Contract.Publishing
{
    /// <summary>
    /// Manages the style setting for visualize data on the PyxisView::Globe.
    /// </summary>
    public class Style
    {
        /// <summary>
        /// Gets or sets the fill style to use in areas.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public FieldStyle Fill { get; set; }

        /// <summary>
        /// Gets or sets the line style.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public FieldStyle Line { get; set; }

        /// <summary>
        /// Gets or sets the icon style.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public IconFieldStyle Icon { get; set; }

        /// <summary>
        /// List of optional effects
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<StyleEffect> Effects { get; set; }

        /// <summary>
        /// Gets or sets if the data should be treated as elevation
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public bool? ShowAsElevation { get; set; }
    }

    /// <summary>
    /// Options available to style Pyxis.UI.Layers.Globe.FieldStyle.
    /// </summary>
    public enum FieldStyleOptions
    {
        /// <summary>
        /// No style.
        /// </summary>
        None,
        /// <summary>
        /// Style using a solid color.
        /// </summary>
        SolidColor,
        /// <summary>
        /// Style using a Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        Palette
    }

    /// <summary>
    /// A palette consisting of discrete steps each with fixed color.
    /// Palettes can be numeric or lexicographic.
    /// </summary>
    public class Palette
    {
        /// <summary>
        /// Defines a the System.Drawing.Color to use for values falling between the step's value and the value of the next ordered step.
        /// </summary>
        public class Step
        {
            /// <summary>
            /// Gets or sets the value.
            /// </summary>
            public object Value { get; set; }

            /// <summary>
            /// Gets or sets the System.Drawing.Color.
            /// </summary>
            [JsonConverter(typeof(ColorJsonConverter))]
            public Color Color { get; set; }
        }

        /// <summary>
        /// The Pyxis.UI.Layers.Globe.Palette.Steps in the Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        public List<Step> Steps { get; set; }

        /// <summary>
        /// Create an equally-spaced Pyxis.UI.Layers.Globe.Palette from an array of System.Drawing.Color.
        /// The Steps in the created Pyxis.UI.Layers.Globe.Palette are between 0 and 1.
        /// </summary>
        /// <param name="colors">System.Drawing.Color to use at each equally-spaced step.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.Palette.</returns>
        public static Palette Create(params Color[] colors)
        {
            var palette = new Palette() { Steps = new List<Step>() };

            double i = 0.0;
            double divider = Math.Max(1.0, colors.Length - 1);
            foreach (var color in colors)
            {
                palette.Steps.Add(new Step() { Color = color, Value = i / divider });
                i += 1;
            }

            return palette;
        }

        public Palette Add(Color color, object value)
        {
            if (Steps == null)
            {
                Steps = new List<Step>();
            }
            Steps.Add(new Step() {Color = color, Value = value});
            return this;
        }

        public Palette Add(string color, object value)
        {
            return Add(CssColorConverter.FromCss(color), value);
        }

        public FieldStyle AsFieldStyle(string expression)
        {
            return FieldStyle.FromPalette(expression,this);
        }
    }

    /// <summary>
    /// A Generic class to store an additional style effects.
    /// 
    /// An Effect is identified by a name: "Blur", "Pattern", "Shadow", etc.
    /// It is up to the rendering engine to implement/support effects.
    /// If an effect is not supported it should be ignored.
    /// </summary>
    public class StyleEffect
    {
        /// <summary>
        /// Type of the effect
        /// </summary>
        public string Type { get; set; }

        /// <summary>
        /// Properties of the effect
        /// </summary>
        public Dictionary<string,object> Properties { get; set; } 
    }

    /// <summary>
    /// Encapsulates the style of a field.
    /// </summary>
    public class FieldStyle
    {
        /// <summary>
        /// Gets or sets the expression used to apply the palette.
        /// </summary>
        public string PaletteExpression { get; set; }
        /// <summary>
        /// Gets or sets the type of the style.
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        public FieldStyleOptions Style { get; set; }
        /// <summary>
        /// Gets or sets the Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        public Palette Palette { get; set; }

        /// <summary>
        /// Gets or sets the System.Drawing.Color.
        /// </summary>
        [JsonConverter(typeof(ColorJsonConverter))]
        public Color? Color { get; set; }

        /// <summary>
        /// List of optional effects
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<StyleEffect> Effects { get; set; }

        /// <summary>
        /// Gets an instance of Pyxis.UI.Layers.Globe.FieldStyle without styling.
        /// </summary>
        public static FieldStyle NoStyle
        {
            get
            {
                return new FieldStyle()
                {
                    Style = FieldStyleOptions.None,
                };
            }
        }

        /// <summary>
        /// Creates a Pyxis.UI.Layers.Globe.FieldStyle from a System.Drawing.Color.
        /// </summary>
        /// <param name="color">The System.Drawing.Color to use.</param>
        /// <returns>A Pyxis.UI.Layers.Globe.FieldStyle with solid color set using <paramref name="color"/>.</returns>
        public static FieldStyle FromColor(Color color)
        {
            return new FieldStyle()
            {
                Style = FieldStyleOptions.SolidColor,
                Color = color
            };
        }

        /// <summary>
        /// Creates a Pyxis.UI.Layers.Globe.FieldStyle from a Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        /// <param name="paletteExpression">The expression for applying <paramref name="palette"/>.</param>
        /// <param name="palette">Pyxis.UI.Layers.Globe.Palette to style the field.</param>
        /// <returns>A Pyxis.UI.Layers.Globe.FieldStyle with palette styling set using <paramref name="palette"/>.</returns>
        public static FieldStyle FromPalette(string paletteExpression, Palette palette)
        {
            return new FieldStyle()
            {
                Style = FieldStyleOptions.Palette,
                PaletteExpression = paletteExpression,
                Palette = palette
            };
        }

        public static bool operator ==(FieldStyle a, FieldStyle b)
        {
            //speed up for null elements
            if (Equals(a, null))
            {
                return Equals(b, null);
            }
            if (Equals(b, null))
            {
                return false;
            }

            return JsonConvert.SerializeObject(a) == JsonConvert.SerializeObject(b);
        }

        public static bool operator !=(FieldStyle a, FieldStyle b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            var other = obj as FieldStyle;

            if (other == null) return false;

            return this == other;
        }

        public override int GetHashCode()
        {
            var hash = Style.GetHashCode();
            if (Color.HasValue)
            {
                hash ^= Color.GetHashCode();
            }
            if (Palette != null)
            {
                hash ^= Palette.GetHashCode();
            }
            if (PaletteExpression != null)
            {
                hash ^= PaletteExpression.GetHashCode();
            }
            return hash;
        }
    }

    /// <summary>
    /// Encapsulates the style of an icon.
    /// </summary>
    public class IconFieldStyle : FieldStyle
    {
        /// <summary>
        /// Gets or sets the URL of the icon image.
        /// </summary>
        public string IconDataUrl { get; set; }
        /// <summary>
        /// Gets or sets the scale.
        /// </summary>
        public double Scale { get; set; }

        /// <summary>
        /// Gets an instance of Pyxis.UI.Layers.Globe.IconFieldStyle without styling.
        /// </summary>
        public static IconFieldStyle NoIcon
        {
            get
            {
                return new IconFieldStyle() { Style = FieldStyleOptions.None };
            }
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.IconFieldStyle using a System.Drawing.Image.
        /// </summary>
        /// <param name="image">The System.Drawing.Image to use for the icon.</param>
        /// <returns>Pyxis.UI.Layers.Globe.IconStyle created using <paramref name="image"/>.</returns>
        public static IconFieldStyle FromImage(Image image)
        {
            return FromImage(image, System.Drawing.Color.White, 1.0);
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.IconFieldStyle using a System.Drawing.Image.
        /// </summary>
        /// <param name="image">The System.Drawing.Image to use for the icon.</param>
        /// <param name="scale">The desired scale of the icon.</param>
        /// <returns>Pyxis.UI.Layers.Globe.IconFieldStyle created using <paramref name="image"/>.</returns>
        public static IconFieldStyle FromImage(Image image, double scale)
        {
            return FromImage(image, System.Drawing.Color.White, scale);
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.IconFieldStyle using a System.Drawing.Image.
        /// </summary>
        /// <param name="image">The System.Drawing.Image to use for the icon.</param>
        /// <param name="color">The desired System.Drawing.Color of the icon.</param>
        /// <returns>Pyxis.UI.Layers.Globe.IconFieldStyle created using <paramref name="image"/>.</returns>
        public static IconFieldStyle FromImage(Image image, Color color)
        {
            return FromImage(image, color, 1.0);
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.IconFieldStyle using a System.Drawing.Image.
        /// </summary>
        /// <param name="image">The System.Drawing.Image to use for the icon.</param>
        /// <param name="color">The desired System.Drawing.Color of the icon.</param>
        /// <param name="scale">The desired scale of the icon.</param>
        /// <returns>Pyxis.UI.Layers.Globe.IconFieldStyle created using <paramref name="image"/>.</returns>
        public static IconFieldStyle FromImage(Image image, Color color, double scale)
        {
            return new IconFieldStyle()
            {
                IconDataUrl = image.ToDataUrl(),
                Style = FieldStyleOptions.SolidColor,
                Color = color,
                Scale = scale
            };
        }

        /// <summary>
        /// Create a copy of a Pyxis.UI.Layers.Globe.IconFieldStyle with a different Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        /// <param name="basedOn">The Pyxis.UI.Layers.Globe.IconFieldStyle to base the new style on.</param>
        /// <param name="paletteExpression">The palette expression to use.</param>
        /// <param name="palette">The Pyxis.UI.Layers.Globe.Palette to use.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.IconFieldStyle.</returns>
        public static IconFieldStyle ChangePalette(IconFieldStyle basedOn, string paletteExpression, Palette palette)
        {
            if (basedOn == null)
            {
                throw new ArgumentNullException("basedOn");
            }
            return new IconFieldStyle()
            {
                IconDataUrl = basedOn.IconDataUrl,
                Style = FieldStyleOptions.Palette,
                PaletteExpression = paletteExpression,
                Palette = palette,
                Scale = basedOn.Scale
            };
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.IconFieldStyle with a System.Drawing.Image and a Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        /// <param name="image">The System.Drawing.Image to use for the icon.</param>
        /// <param name="paletteExpression">The palette expression to use.</param>
        /// <param name="palette">The Pyxis.UI.Layers.Globe.Palette to use.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.IconFieldStyle.</returns>
        public static IconFieldStyle FromImageAndPalette(Image image, string paletteExpression, Palette palette)
        {
            return FromImageAndPalette(image, paletteExpression, palette, 1.0);
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.IconFieldStyle with a System.Drawing.Image and a Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        /// <param name="image">The System.Drawing.Image to use for the icon.</param>
        /// <param name="paletteExpression">The palette expression to use.</param>
        /// <param name="palette">The Pyxis.UI.Layers.Globe.Palette to use.</param>
        /// <param name="scale">The desired scale of the icon.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.IconFieldStyle.</returns>
        public static IconFieldStyle FromImageAndPalette(Image image, string paletteExpression, Palette palette, double scale)
        {
            return new IconFieldStyle()
            {
                IconDataUrl = image.ToDataUrl(),
                Style = FieldStyleOptions.Palette,
                PaletteExpression = paletteExpression,
                Palette = palette,
                Scale = scale
            };
        }
    }
}
