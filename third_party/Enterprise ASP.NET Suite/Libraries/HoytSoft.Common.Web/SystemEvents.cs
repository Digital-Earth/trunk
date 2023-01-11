#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using HoytSoft.Common;
using HoytSoft.Common.Web;
using HoytSoft.Common.Configuration;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	///<summary>Used to hear about a system event like a new user being added to the system. It's essentially a listener.</summary>
	public interface ISystemEvent {
		///<summary>When a new user is created. If a new membership is created, then the NewMembership() will fire first, then NewMember().</summary>
		/// <returns>True if successful.</returns>
		bool NewUser(IUser User);
		///<summary>When a user is deleted from the system - it may not be tied to a membership. For instance, a user that's managed by someone else is deleted.</summary>
		/// <returns>True if successful.</returns>
		bool RemoveUser(IUser User);
		
		///<summary>When a user's data is supposed to be cleared out. You can assume it's being asked to be deleted. This is apart from RemoveMember and CancelMembership.</summary>
		/// <returns>True if successful.</returns>
		bool ClearData(IUser User);

		///<summary>When a member logs in or returns to visit the site after being away.</summary>
		/// <returns>True if successful.</returns>
		bool UserLogin(IUser User);

		///<summary>When a member logs out.</summary>
		/// <returns>True if successful.</returns>
		bool UserLogout(IUser User);
	}

	///<seealso cref="ISystemEvent" />
	public abstract class AbstractSystemEvent : ISystemEvent {
		///<seealso cref="ISystemEvent" />
		public virtual bool NewUser(IUser User) { return true; }
		///<seealso cref="ISystemEvent" />
		public virtual bool RemoveUser(IUser User) { return true; }

		///<seealso cref="ISystemEvent" />
		public virtual bool ClearData(IUser User) { return true; }

		///<seealso cref="ISystemEvent" />
		public virtual bool UserLogin(IUser User) { return true; }

		///<seealso cref="ISystemEvent" />
		public virtual bool UserLogout(IUser User) { return true; }
	}

	public class SystemEvents {
		#region Variables
		private static bool loaded = false;
		private static SystemEvents singleton = null;
		private int evtObjectCount = 0;
		private System.Collections.Generic.IList<ISystemEvent> evtObjects = null;
		#endregion

		#region Constructors
		static SystemEvents() {
		}
		#endregion

		#region Properties
		public static System.Collections.Generic.IList<ISystemEvent> SystemEventObjects { get { if (!loaded) Reload(); if (singleton != null) return singleton.evtObjects; else return null; } }
		public static int SystemEventObjectCount { get { if (!loaded) Reload(); if (singleton != null) return singleton.evtObjectCount; else return 0; } }
		#endregion

		#region SystemEvent Methods
		public static void Reload() {
			loaded = true;
			singleton = new SystemEvents();

			//Causes the settings to dump SystemEvent handlers into the singleton object
			//through calling AddObject() in the Create() method.
			SystemEventsSettings settings = (SystemEventsSettings)Settings.From(Settings.Section.SystemEvents);
		}

		public static void AddObject(ISystemEvent Obj) {
			if (!loaded) Reload();
			if (Obj == null || singleton == null) return;
			if (singleton.evtObjects == null)
				singleton.evtObjects = new System.Collections.Generic.List<ISystemEvent>();
			singleton.evtObjects.Add(Obj);
			++singleton.evtObjectCount;
		}

		public static void RemoveObject(ISystemEvent Obj) {
			if (!loaded) Reload();
			if (Obj == null || singleton == null || singleton.evtObjects == null || !singleton.evtObjects.Contains(Obj)) return;
			singleton.evtObjects.Remove(Obj);
		}

		public static void ClearObjects(ISystemEvent Obj) {
			if (!loaded) Reload();
			if (Obj == null || singleton == null || singleton.evtObjects == null) return;
			singleton.evtObjects.Clear();
		}

		public static bool UserLogin(IUser User) {
			if (User == null) return false;
			if (SystemEventObjects == null || SystemEventObjectCount <= 0) return true;
			bool ret = true;
			foreach (ISystemEvent e in SystemEventObjects)
				if (!e.UserLogin(User))
					ret = false;
			return ret;
		}

		public static bool UserLogout(IUser User) {
			if (User == null) return false;
			if (SystemEventObjects == null || SystemEventObjectCount <= 0) return true;
			bool ret = true;
			foreach (ISystemEvent e in SystemEventObjects)
				if (!e.UserLogout(User))
					ret = false;
			return ret;
		}

		public static bool NewUser(IUser User) {
			if (User == null) return false;
			if (SystemEventObjects == null || SystemEventObjectCount <= 0) return true;
			bool ret = true;
			foreach (ISystemEvent e in SystemEventObjects)
				if (!e.NewUser(User))
					ret = false;
			return ret;
		}

		public static bool RemoveUser(IUser User) {
			if (User == null) return false;
			if (SystemEventObjects == null || SystemEventObjectCount <= 0) return true;
			bool ret = true;
			foreach (ISystemEvent e in SystemEventObjects)
				if (!e.RemoveUser(User))
					ret = false;
			return ret;
		}

		public static bool ClearData(IUser User) {
			if (User == null) return false;
			if (SystemEventObjects == null || SystemEventObjectCount <= 0) return true;
			bool ret = true;
			foreach (ISystemEvent e in SystemEventObjects)
				if (!e.ClearData(User))
					ret = false;
			return ret;
		}
		#endregion
	}
}
