using System;
using System.Collections.Generic;
using HoytSoft.Common.Configuration;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	public class OnePageWonders {
		public static Wonder FindForDomain(string Domain) {
			OnePageWondersSettings settings = Settings.From<OnePageWondersSettings>(Settings.Section.OnePageWonders);
			if (settings == null)
				return null;

			if (settings.OnePageWonderObjects == null)
				return null;

			foreach (Wonder p in settings.OnePageWonderObjects.Values)
				if (p.ContainsDomain(Domain))
					return p;

			return null;
		}
	}

	#region Helper Classes
	public class Wonder {
		private List<string> domains;
		private string name;
		private string pageOrig, pageResolved;

		public Wonder(string Name, string OriginalPage, string ResolvedPage, string[] Domains) {
			this.name = Name;
			this.pageOrig = OriginalPage;
			this.pageResolved = ResolvedPage;
			this.domains = new List<string>(Domains);
		}

		public string Name { get { return this.name; } }
		public string OriginalPage { get { return this.pageOrig; } }
		public string ResolvedPage { get { return this.pageResolved; } }
		public List<string> Domains { get { return this.domains; } }

		public bool ContainsDomain(string Domain) {
			if (this.domains == null) return false;
			return this.domains.Contains(Domain);
		}
	}
	#endregion
}
