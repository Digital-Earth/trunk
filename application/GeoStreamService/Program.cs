/******************************************************************************
Program.cs

begin		: May 13, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using HoytSoft.Common.Services;

namespace GeoStreamService
{
    internal class Program
    {
        private static void Main(string[] args)
        {
            // Explicitly specify what to run
            ServiceBase.RunService(args, typeof(GeoStreamService));
        }
    }
}