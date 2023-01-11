#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Gateways {
	public interface IGateway {
		Version Version { get; }
	}

	public abstract class AbstractGateway : IGateway {
		public abstract Version Version { get; }
	}
}
