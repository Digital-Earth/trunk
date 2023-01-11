#include "TextHost.h"
#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Diagnostics;
using namespace System::Drawing;
using namespace System::Drawing::Drawing2D;
using namespace System::Reflection;

namespace Infralution {
    namespace RichText {


        /// <summary>
        /// Defines a component for rendering Rich Text to a Graphics context
        /// </summary>
        public ref class RichTextRenderer :  public System::ComponentModel::Component
        {
        public:

            /// <summary>
            /// Create a new instance of a RichTextRenderer
            /// </summary>
            RichTextRenderer(void)
            {
#ifdef CHECK_PUBLIC_KEY
                CheckCallingAssembly(Assembly::GetCallingAssembly());
#endif
                InitializeComponent();
                horizontalAlignment = StringAlignment::Near;
                verticalAlignment = StringAlignment::Near;
                textColor = Drawing::SystemColors::WindowText;
                textHost = new CTextHost();
            }

            /// <summary>
            /// Required for Windows.Forms Class Composition Designer support
            /// </summary>
            RichTextRenderer(System::ComponentModel::IContainer ^container)
            {
                container->Add(this);
                InitializeComponent();
            }
        
            /// <summary>
            /// The RTF text to be rendered
            /// </summary>
            property String^ Text 
            {
                virtual String^ get() { return text; }
                virtual void set(String^ value) 
                { 
                    // don't check if the value is the same as the existing text
                    // because the rich text renderer behaves strangely (loses formatting)
                    // if the text is not set after changing the base properties (such as text 
                    // color and font)
                    //
                    text = value; 
                    textHost->SetText(text);
                }
            }
            
            /// <summary>
            /// Should the text be word wrapped
            /// </summary>
            property bool WordWrap 
            {
                virtual bool get() { return textHost->WordWrap(); }
                virtual void set(bool value) 
                { 
                    textHost->SetWordWrap(value);
                }
            }

            /// <summary>
            /// Should the text be multiline
            /// </summary>
            property bool MultiLine 
            {
                virtual bool get() { return textHost->MultiLine(); }
                virtual void set(bool value) 
                { 
                    textHost->SetMultiLine(value);
                }
            }

            /// <summary>
            /// The default font for the RichText
            /// </summary>
            property Drawing::Font^ Font 
            {
                virtual Drawing::Font^ get() 
                {
                    if (!font)
                        font = textHost->DefaultFont();
                    return font; 
                }
                virtual void set(Drawing::Font^ value) 
                { 
                    if (!value) throw gcnew System::ArgumentNullException();
                    if (value != font)
                    {
                        font = value;
                        textHost->SetFont(value);
                    }
                }
            }

            /// <summary>
            /// The default horizontal aligment for the RichText
            /// </summary>
            property StringAlignment HorizontalAlignment 
            {
                virtual StringAlignment get() { return horizontalAlignment; }
                virtual void set(StringAlignment value) 
                { 
                    if (value != horizontalAlignment)
                    {
                        horizontalAlignment = value;
                        switch (value)
                        {
                        case StringAlignment::Near:
                            textHost->SetHorizontalAlignment(PFA_LEFT);
                            break;
                        case StringAlignment::Far:
                            textHost->SetHorizontalAlignment(PFA_RIGHT);
                            break;
                        case StringAlignment::Center:
                            textHost->SetHorizontalAlignment(PFA_CENTER);
                            break;
                        }
                    }
                }
            }

            /// <summary>
            /// The default vertical aligment for the RichText
            /// </summary>
            property StringAlignment VerticalAlignment 
            {
                virtual StringAlignment get() { return verticalAlignment; }
                virtual void set(StringAlignment value) 
                { 
                        verticalAlignment = value;
                }
            }

            /// <summary>
            /// The default text color for the RichText
            /// </summary>
            property Drawing::Color TextColor 
            {
                virtual Drawing::Color get() { return textColor; }
                virtual void set(Drawing::Color value) 
                { 
                    if (value != textColor)
                    {
                        textColor = value;
                        textHost->SetTextColor(RGB(value.R, value.G, value.B));
                    }
                }
            }

            /// <summary>
            /// Draw the current text to the given graphics context
            /// </summary>
            void Draw(Graphics^ graphics, int x, int y)
            {
                // transform the point to device coordinates
                //
			    array<Point> ^points = { Point(x, y) };
                graphics->TransformPoints(CoordinateSpace::Device, CoordinateSpace::World, points);

                IntPtr hdc = graphics->GetHdc();
                HDC phdc = (HDC)hdc.ToPointer();
                try
                {
                    RECT rect;
                    rect.left = points[0].X;
                    rect.top = points[0].Y;
                    rect.right = textHost->GetNaturalWidth(phdc);
                    rect.bottom = textHost->GetNaturalHeight(phdc, 1);
                    textHost->Draw(phdc, &rect);
                }
                finally
                {
                    graphics->ReleaseHdc(hdc);
                } 
            }

            /// <summary>
            /// Draw the current text to the given rectangle of the graphics context
            /// </summary>
            void Draw(Graphics^ graphics, Drawing::Rectangle r)
            {
                // convert the rectangle to device coordinates
                //
                Drawing::Rectangle dr;
			    array<Point> ^points = { r.Location, Point(r.Right, r.Bottom) };
                graphics->TransformPoints(CoordinateSpace::Device, CoordinateSpace::World, points);
			    dr.Location = points[0];
			    dr.Width = points[1].X - points[0].X + 1;
			    dr.Height = points[1].Y - points[0].Y + 1;

                IntPtr hdc = graphics->GetHdc();
                HDC phdc = (HDC)hdc.ToPointer();
                try
                {

                    RECT rect;
                    rect.left = dr.Left;
                    rect.top = dr.Top;
                    rect.right = dr.Right;
                    rect.bottom = dr.Bottom;

                    // handle vertical alignment ourselves
                    //
                    if (verticalAlignment != StringAlignment::Near)
                    {
                        int height = textHost->GetNaturalHeight(phdc, dr.Width);
                        if (verticalAlignment == StringAlignment::Center)
                        {
                            rect.top += (dr.Height - height) / 2;
                        }
                        else if (verticalAlignment == StringAlignment::Far)
                        {
                            rect.top += (dr.Height - height);
                        }
                    }

                    textHost->Draw(phdc, &rect);
                }
                finally
                {
                    graphics->ReleaseHdc(hdc);
                }
            }

            /// <summary>
            /// Return the natural width of the current text
            /// </summary>
            int GetNaturalWidth(Graphics^ graphics)
            {
                int width = 0;
                IntPtr hdc = graphics->GetHdc();
                try
                {
                    width = textHost->GetNaturalWidth((HDC)hdc.ToPointer());
                }
                finally
                {
                    graphics->ReleaseHdc(hdc);
                }

                // transform the width into world coordinates
                //
			    array<Point> ^points = { Point(width, 0) };
                graphics->TransformPoints(CoordinateSpace::World, CoordinateSpace::Device, points);
                return points[0].X;
           }

            /// <summary>
            /// Return the natural height of the current text
            /// </summary>
            int GetNaturalHeight(Graphics^ graphics, int width)
            {
                // transform the width into device coords
                //
                array<Point> ^points = { Point(width, 0) };
                graphics->TransformPoints(CoordinateSpace::Device, CoordinateSpace::World, points);
                width = points[0].X;

                int height = 0;
                IntPtr hdc = graphics->GetHdc();
                try
                {
                    height = textHost->GetNaturalHeight((HDC)hdc.ToPointer(), width);
                }
                finally
                {
                    graphics->ReleaseHdc(hdc);
                }

                // transform the height into world coordinates
                //
                points[0] = Point(0, height) ;
                graphics->TransformPoints(CoordinateSpace::World, CoordinateSpace::Device, points);
                return points[0].Y;
           }

        protected:

            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~RichTextRenderer()
            {
                if (textHost)
                {
                    delete textHost;
                }

                if (components)
                {
                    delete components;
                }
            }

        private:

            /// <summary>
            /// Check the calling assembly is an Infralution assembly
            /// </summary>
            void CheckCallingAssembly(Assembly^ assembly)
            {
                array<byte> ^token = assembly->GetName()->GetPublicKeyToken();
                array<byte> ^requiredToken = { 0x3E, 0x7E, 0x8E, 0x37, 0x44, 0xA5, 0xC1, 0x3F };  

                bool validToken = (token) ? true: false;
                if (validToken)
                {
                    validToken = (token->Length == requiredToken->Length);
                    if (validToken)
                    {
                        for(int i=0; i < token->Length; i++) 
                        {
                            if (token[i] != requiredToken[i])
                            {
                                validToken = false;
                                break;
                            }
                        }
                    }
                }
                if (!validToken)
                {
                    throw gcnew System::UnauthorizedAccessException("This class can only be used by Infralution assemblies");
                }
            }


#pragma region Windows Form Designer generated code
 
            /// <summary>
            /// Required method for Designer support - do not modify
            /// the contents of this method with the code editor.
            /// </summary>
            void InitializeComponent(void)
            {
                components = gcnew System::ComponentModel::Container();
            }

#pragma endregion

            /// <summary>
            /// Required designer variable.
            /// </summary>
            System::ComponentModel::Container ^components;

            /// <summary>
            /// Unmanaged class that implements the ITextHost interface.
            /// </summary>
            CTextHost* textHost;
            
            /// <summary>
            /// The current text to be rendered
            /// </summary>
            String^ text;

            /// <summary>
            /// The default font
            /// </summary>
            Drawing::Font^ font;

            /// <summary>
            /// The default horizontal Alignment
            /// </summary>
            StringAlignment horizontalAlignment;

            /// <summary>
            /// The default vertical Alignment
            /// </summary>
            StringAlignment verticalAlignment;

            /// <summary>
            /// The default text color
            /// </summary>
            Drawing::Color textColor;
        };
    }
}