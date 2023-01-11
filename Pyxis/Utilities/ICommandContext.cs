/******************************************************************************
ICommandContext.cs

begin      : October 19, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Interface for a command context.  By default a context doesn't do 
    /// anything, other than identify an object as a context.  In many cases 
    /// we will want to derive more interesting context interfaces from this
    /// one (IProcessCommandContext, IGlobeControlCommandContext, etc.)
    /// </summary>
    /// <remarks>
    /// See https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=User_Story/Command_Infrastructure"
    ///</remarks>
    public interface ICommandContext
    {
    }
}
