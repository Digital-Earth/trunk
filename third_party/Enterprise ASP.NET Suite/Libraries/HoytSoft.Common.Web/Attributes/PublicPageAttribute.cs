#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Reflection;

namespace HoytSoft.Common.Web {

	///<summary>Marker class that prevents a HoytSoft.Common.Page from checking if the user's authenticated.</summary>
	[AttributeUsage(AttributeTargets.Class, AllowMultiple=false, Inherited=true)]
	public class PublicPageAttribute : System.Attribute { }

}
