// LVICompare.cs
// Copyright (C) 2002 Matt Zyzik (www.FileScope.com)
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

using System;
using System.Collections;
using System.Windows.Forms;

namespace FileScope
{
    /// <summary>
    /// Compare two listview items.
    /// </summary>
    public class LVICompare
    {
        /// <summary>
        /// Returns true if the subitems are the same.
        /// </summary>
        public static bool Compare(ListViewItem lvi, string[] subitems)
        {
            if (lvi.SubItems.Count != subitems.Length)
                return false;
            for (int x = 0; x < lvi.SubItems.Count; x++)
                if (lvi.SubItems[x].ToString() != subitems[x])
                    return false;
            return true;
        }
    }
}
