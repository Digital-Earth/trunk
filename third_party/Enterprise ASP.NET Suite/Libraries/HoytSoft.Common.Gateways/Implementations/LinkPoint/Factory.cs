#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public sealed class Factory {
		public static LinkPointGateway CreateNew() {
			return CreateNew(SupportedVersion.Default);
		}

		public static LinkPointGateway CreateNew(SupportedVersion Version) {
			switch (Version) {
				case SupportedVersion.V3_5:
					return new LinkPointGatewayV3_5();
				case SupportedVersion.V4_0:
					return null;
				case SupportedVersion.Default:
					return new LinkPointGatewayV3_5();
			}
			return null;
		}
	}
}
