﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace StudioLauncher.Properties {
    
    
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.VisualStudio.Editors.SettingsDesigner.SettingsSingleFileGenerator", "11.0.0.0")]
    internal sealed partial class Settings : global::System.Configuration.ApplicationSettingsBase {
        
        private static Settings defaultInstance = ((Settings)(global::System.Configuration.ApplicationSettingsBase.Synchronized(new Settings())));
        
        public static Settings Default {
            get {
                return defaultInstance;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("")]
        public string LocalStudioVersion {
            get {
                return ((string)(this["LocalStudioVersion"]));
            }
            set {
                this["LocalStudioVersion"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("https://ls-api.globalgridsystems.com/api/v1")]
        public string RestAPIVersionServerAddress {
            get {
                return ((string)(this["RestAPIVersionServerAddress"]));
            }
            set {
                this["RestAPIVersionServerAddress"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("https://www.globalgridsystems.com/studio/beta-v1")]
        public string ProductionStartupURL {
            get {
                return ((string)(this["ProductionStartupURL"]));
            }
            set {
                this["ProductionStartupURL"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("https://www-test.globalgridsystems.com/studio/beta-v1")]
        public string TestStartupURL {
            get {
                return ((string)(this["TestStartupURL"]));
            }
            set {
                this["TestStartupURL"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("https://www-dev.globalgridsystems.com/studio/beta-v1")]
        public string DevelopmentStartupURL {
            get {
                return ((string)(this["DevelopmentStartupURL"]));
            }
            set {
                this["DevelopmentStartupURL"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("startup")]
        public string StartupArg {
            get {
                return ((string)(this["StartupArg"]));
            }
            set {
                this["StartupArg"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("False")]
        public bool ControlMode {
            get {
                return ((bool)(this["ControlMode"]));
            }
            set {
                this["ControlMode"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("https://www-staging.globalgridsystems.com/studio/beta-v1")]
        public string StagingStartupURL {
            get {
                return ((string)(this["StagingStartupURL"]));
            }
            set {
                this["StagingStartupURL"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("testdirectory")]
        public string TestDirectoryArg {
            get {
                return ((string)(this["TestDirectoryArg"]));
            }
            set {
                this["TestDirectoryArg"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("clearcache")]
        public string ClearCacheArg {
            get {
                return ((string)(this["ClearCacheArg"]));
            }
            set {
                this["ClearCacheArg"] = value;
            }
        }
    }
}
