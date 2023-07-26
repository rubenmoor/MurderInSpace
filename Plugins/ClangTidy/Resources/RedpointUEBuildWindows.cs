// Copyright Redpoint Games. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;
using Microsoft.Win32;
using System.Linq;
#if NETCOREAPP
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif
using Microsoft.VisualStudio.Setup.Configuration;
using System.Runtime.InteropServices;
#if NETCOREAPP
using System.Reflection;
#endif
#if NET6_0
using Microsoft.Extensions.Logging;
#endif

#if NETCOREAPP
#nullable disable
#endif

namespace UnrealBuildTool
{
    class RedpointWindowsPlatform : WindowsPlatform
    {
#if NETCOREAPP
#if NET6_0
		public RedpointWindowsPlatform(UnrealTargetPlatform InPlatform, MicrosoftPlatformSDK InSDK, ILogger InLogger) : base(InPlatform, InSDK, InLogger)
#else
		public RedpointWindowsPlatform(UnrealTargetPlatform InPlatform, MicrosoftPlatformSDK InSDK) : base(InPlatform, InSDK)
#endif
#else
        public RedpointWindowsPlatform(UnrealTargetPlatform InPlatform, WindowsPlatformSDK InSDK) : base(InPlatform, InSDK)
#endif
        {
        }

        public override void GetExternalBuildMetadata(FileReference ProjectFile, StringBuilder Metadata)
        {
            base.GetExternalBuildMetadata(ProjectFile, Metadata);

            var IsLiveCoding = Environment.GetCommandLineArgs().Select(x => x.ToLowerInvariant()).Contains("-livecoding");
            if (IsLiveCoding)
            {
                Metadata.AppendLine("IsLiveCoding");
            }
            else
            {
                Metadata.AppendLine("IsNotLiveCoding");
            }
        }

        public override UEToolChain CreateToolChain(ReadOnlyTargetRules Target)
        {
#if NET6_0
            if (Target.StaticAnalyzer == StaticAnalyzer.PVSStudio)
#else
            if (Target.WindowsPlatform.StaticAnalyzer == WindowsStaticAnalyzer.PVSStudio)
#endif
            {
                return base.CreateToolChain(Target);
            }

#if NET6_0
            return new ClangTidyToolChain(Target, Logger);
#else
            return new ClangTidyToolChain(Target);
#endif
        }
    }

    class ZZZ_RedpointWindowsPlatformFactory : UEBuildPlatformFactory
    {
        public override UnrealTargetPlatform TargetPlatform
        {
            get { return UnrealTargetPlatform.Win64; }
        }

        /// <summary>
        /// Register the platform with the UEBuildPlatform class
        /// </summary>
#if NET6_0
        public override void RegisterBuildPlatforms(ILogger logger)
#else
        public override void RegisterBuildPlatforms()
#endif
        {
            if (Environment.GetEnvironmentVariable("UBT_CLANG_TIDY_EXTENSIONS") == "false")
            {
                // If you set the environment variable UBT_CLANG_TIDY_EXTENSIONS=false, then clang-tidy extensions will
                // not be registered.
                return;
            }

#if NETCOREAPP
#if NET6_0
			MicrosoftPlatformSDK SDK = new MicrosoftPlatformSDK(logger);
#else
			MicrosoftPlatformSDK SDK = new MicrosoftPlatformSDK();
#endif
#else
            WindowsPlatformSDK SDK = new WindowsPlatformSDK();
            SDK.ManageAndValidateSDK();
#endif

            // Unset the Windows platforms...
            Dictionary<UnrealTargetPlatform, UEBuildPlatform> Dict = (Dictionary<UnrealTargetPlatform, UEBuildPlatform>)typeof(UEBuildPlatform).GetField("BuildPlatformDictionary", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static).GetValue(null);
            Dict.Remove(UnrealTargetPlatform.Win64);
#if !NETCOREAPP
            Dict.Remove(UnrealTargetPlatform.Win32);
#endif

#if NETCOREAPP
			Dictionary<string, UEBuildPlatformSDK> SDKRegistry = (Dictionary<string, UEBuildPlatformSDK>)typeof(UEBuildPlatformSDK).GetField("SDKRegistry", BindingFlags.NonPublic | BindingFlags.Static).GetValue(null);
			SDKRegistry.Remove(UnrealTargetPlatform.Win64.ToString());
#endif

            // Register this build platform for both Win64 and Win32
#if NET6_0
            UEBuildPlatform.RegisterBuildPlatform(new RedpointWindowsPlatform(UnrealTargetPlatform.Win64, SDK, logger), logger);
#else
            UEBuildPlatform.RegisterBuildPlatform(new RedpointWindowsPlatform(UnrealTargetPlatform.Win64, SDK));
#endif
            UEBuildPlatform.RegisterPlatformWithGroup(UnrealTargetPlatform.Win64, UnrealPlatformGroup.Windows);
            UEBuildPlatform.RegisterPlatformWithGroup(UnrealTargetPlatform.Win64, UnrealPlatformGroup.Microsoft);
            UEBuildPlatform.RegisterPlatformWithGroup(UnrealTargetPlatform.Win64, UnrealPlatformGroup.Desktop);

#if !NETCOREAPP
            UEBuildPlatform.RegisterBuildPlatform(new RedpointWindowsPlatform(UnrealTargetPlatform.Win32, SDK));
            UEBuildPlatform.RegisterPlatformWithGroup(UnrealTargetPlatform.Win32, UnrealPlatformGroup.Windows);
            UEBuildPlatform.RegisterPlatformWithGroup(UnrealTargetPlatform.Win32, UnrealPlatformGroup.Microsoft);
            UEBuildPlatform.RegisterPlatformWithGroup(UnrealTargetPlatform.Win32, UnrealPlatformGroup.Desktop);
#endif

            Log.TraceInformation("Hooked Windows platform with clang-tidy extensions...");
        }
    }
}