// Copyright Redpoint Games. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
#if NETCOREAPP
using EpicGames.Core;
using UnrealBuildBase;
using System.Security.Cryptography;
#else
using Tools.DotNETCommon;
#endif
using Microsoft.Win32;
using System.Net;
using System.Text;
#if NET6_0
using Microsoft.Extensions.Logging;
#endif

#if NETCOREAPP
#nullable disable
#endif

namespace UnrealBuildTool
{
	class ClangTidyToolChain : VCToolChain
	{
		FileReference ClangTidyAnalyzerFile;
		FileReference ClangClCompilerFile;
		FileReference ClangTidySystemLuaFile;
		FileReference ClangQueryFile;
		bool ShouldEmitDiagnostics;
		bool IsLiveCoding;
		HashSet<string> KnownClangLuaPaths;

		static bool bHasIssuedLicenseNotice = false;

#if NET6_0
		public ClangTidyToolChain(ReadOnlyTargetRules Target, ILogger InLogger) : base(Target, InLogger)
#else
		public ClangTidyToolChain(ReadOnlyTargetRules Target) : base(Target)
#endif
		{
			string InstallFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), "ClangTidyForUnrealEngine");
			ClangTidyAnalyzerFile = new FileReference(Path.Combine(InstallFolder, "clang-tidy.exe"));
			ClangClCompilerFile = new FileReference(Path.Combine(InstallFolder, "clang-cl.exe"));
			ClangTidySystemLuaFile = new FileReference(Path.Combine(InstallFolder, "ClangTidySystem.lua"));
			ClangQueryFile = new FileReference(Path.Combine(InstallFolder, "clang-query.exe"));
			KnownClangLuaPaths = new HashSet<string>();

			if (!File.Exists(ClangTidyAnalyzerFile.FullName))
			{
				Log.TraceWarning("Unable to locate clang-tidy. Ensure the plugin has copied across the required files successfully. Expected to find clang-tidy installed at: '{0}'. clang-tidy checks will not run.", ClangTidyAnalyzerFile);
				ClangTidyAnalyzerFile = null;
				ClangClCompilerFile = null;
				ClangTidySystemLuaFile = null;
			}
			else if (!File.Exists(ClangClCompilerFile.FullName))
			{
				Log.TraceWarning("Unable to locate clang-cl. Ensure the plugin has copied across the required files successfully. Expected to find clang-tidy installed at: '{0}'. clang-tidy checks will not run.", ClangClCompilerFile);
				ClangTidyAnalyzerFile = null;
				ClangClCompilerFile = null;
				ClangTidySystemLuaFile = null;
			}
			else if (!File.Exists(ClangTidySystemLuaFile.FullName))
			{
				Log.TraceWarning("Unable to locate ClangTidySystem.lua. Ensure the plugin has copied across the required files successfully. Expected to find ClangTidySystem.lua installed at: '{0}'. clang-tidy checks will not run.", ClangTidySystemLuaFile);
				ClangTidyAnalyzerFile = null;
				ClangClCompilerFile = null;
				ClangTidySystemLuaFile = null;
			}

			ShouldEmitDiagnostics = ((Registry.GetValue(@"HKEY_CURRENT_USER\SOFTWARE\RedpointGames\LicenseManager\ClangTidy", "EmitDiagnostics", "false") as string) ?? "false") == "true";

			IsLiveCoding = Environment.GetCommandLineArgs().Select(x => x.ToLowerInvariant()).Contains("-livecoding");
			if (IsLiveCoding)
			{
				Log.TraceInformation("clang-tidy will not run because this is a Live Coding compile operation.");
			}

			if (!bHasIssuedLicenseNotice)
			{
				Log.TraceInformation("NOTE: clang-tidy is only free for organisations with 5 or less employees. If you are a group of individuals working in a team without a formal company entity, you must have 5 or less people working on your project. If you have more than 5 employees in your organisation or more than 5 people working on your project (in total, not just those using clang-tidy), you MUST obtain a license to cover your usage by emailing sales@redpoint.games.");
				bHasIssuedLicenseNotice = true;
			}
		}

		public override void GetVersionInfo(List<string> Lines)
		{
			base.GetVersionInfo(Lines);

			if (ClangTidyAnalyzerFile != null)
			{
				Lines.Add(String.Format("Using clang-tidy executable at {0}", ClangTidyAnalyzerFile));
			}
		}

#if NETCOREAPP
		class ActionGraphCapture : ForwardingActionGraphBuilder
		{
			List<IExternalAction> Actions;

			public ActionGraphCapture(IActionGraphBuilder Inner, List<IExternalAction> Actions)
				: base(Inner)
			{
				this.Actions = Actions;
			}

			public override void AddAction(IExternalAction Action)
			{
				base.AddAction(Action);
				Actions.Add(Action);
			}
		}

		class ActionGraphCaptureOnly : ForwardingActionGraphBuilder
		{
			List<IExternalAction> Actions;

			public ActionGraphCaptureOnly(IActionGraphBuilder Inner, List<IExternalAction> Actions)
				: base(Inner)
			{
				this.Actions = Actions;
			}

			public override void AddAction(IExternalAction Action)
			{
				Actions.Add(Action);
			}
		}
#else
        class ActionGraphCapture : ForwardingActionGraphBuilder
        {
            List<Action> Actions;

            public ActionGraphCapture(IActionGraphBuilder Inner, List<Action> Actions)
                : base(Inner)
            {
                this.Actions = Actions;
            }

            public override Action CreateAction(ActionType Type)
            {
                Action Action = base.CreateAction(Type);
                Actions.Add(Action);
                return Action;
            }
        }

        class ActionGraphCaptureOnly : ForwardingActionGraphBuilder
        {
            List<Action> Actions;

            public ActionGraphCaptureOnly(IActionGraphBuilder Inner, List<Action> Actions)
                : base(Inner)
            {
                this.Actions = Actions;
            }

            public override Action CreateAction(ActionType Type)
            {
                Action Action = new Action(Type);
                Actions.Add(Action);
                return Action;
            }
        }
#endif

		const string ClangVariantSuffix = "_CT";

		private FileReference GetClangVariantFilename(FileReference Reference)
		{
			var newPath = Path.Combine(Reference.Directory.FullName, ClangVariantSuffix, Reference.GetFileName());
			if (ShouldEmitDiagnostics)
			{
				Console.WriteLine("GetClangVariantFilename: Calculated clang variant filename: " + Reference.FullName + " -> " + newPath);
			}
			return new FileReference(newPath);
		}

		private bool IsClangVariantFilename(FileReference Reference)
		{
			return Reference.ContainsName(ClangVariantSuffix, 0);
		}

		private bool IsPCHFile(FileReference Reference)
		{
			return Reference.HasExtension(".pch") && !IsClangVariantFilename(Reference);
		}

		private bool IsObjFile(FileReference Reference)
		{
			return Reference.HasExtension(".obj") && !IsClangVariantFilename(Reference);
		}

		private bool IsGeneratedCppFile(FileReference Reference)
		{
			return Reference.HasExtension(".cpp") && Reference.FullName.Contains(".gen.") && !IsClangVariantFilename(Reference);
		}

		private bool IsResponseFile(FileReference Reference)
		{
			return Reference.HasExtension(".response") && !IsClangVariantFilename(Reference);
		}

		private bool IsSourceFile(FileReference Reference)
		{
			return (Reference.HasExtension(".c") || Reference.HasExtension(".cc") || Reference.HasExtension(".cpp")) && !IsClangVariantFilename(Reference);
		}

#if NETCOREAPP
		private bool ProducesPCHFile(IExternalAction Action)
#else
        private bool ProducesPCHFile(Action Action)
#endif
		{
			return Action.ProducedItems.Any(x => IsPCHFile(x.Location));
		}

		private static DirectoryReference GetEngineSourceDirectory()
		{
#if NET6_0
			return Unreal.EngineSourceDirectory;
#else
            return UnrealBuildTool.EngineSourceDirectory;
#endif
        }

#if NETCOREAPP
		private FileItem GetProducedPCHFile(IExternalAction Action)
#else
        private FileItem GetProducedPCHFile(Action Action)
#endif
		{
			FileItem File = Action.ProducedItems.FirstOrDefault(x => IsPCHFile(x.Location));
			if (File == null)
			{
				throw new InvalidOperationException("Action does not produce a .pch file.");
			}
			return File;
		}

#if NETCOREAPP
		private FileItem GetProducedObjFile(IExternalAction Action)
#else
        private FileItem GetProducedObjFile(Action Action)
#endif
		{
			FileItem File = Action.ProducedItems.FirstOrDefault(x => IsObjFile(x.Location));
			if (File == null)
			{
				throw new InvalidOperationException("Action does not produce an .obj file.");
			}
			return File;
		}

#if NETCOREAPP
		private FileItem GetPrerequisiteResponseFile(IExternalAction Action)
#else
        private FileItem GetPrerequisiteResponseFile(Action Action)
#endif
		{
			FileItem File = Action.PrerequisiteItems.FirstOrDefault(x => IsResponseFile(x.Location));
			if (File == null)
			{
				throw new InvalidOperationException("Action does not require a .response file.");
			}
			return File;
		}

#if NETCOREAPP
		private FileItem TryGetPrerequisiteSourceFile(IExternalAction Action)
#else
        private FileItem TryGetPrerequisiteSourceFile(Action Action)
#endif
		{
			return Action.PrerequisiteItems.FirstOrDefault(x => IsSourceFile(x.Location));
		}

		private string ComputeInputFileHash(List<FileItem> InputFiles)
		{
#if NETCOREAPP
			using (var Hasher = MD5.Create())
			{
				var NormalizedInputFiles = InputFiles.Select(x => x.FullName.ToUpperInvariant()).OrderBy(x => x).ToList();
				var SB = new StringBuilder();
				for (int i = 0; i < NormalizedInputFiles.Count; i++)
				{
					if (i != 0)
					{
						SB.Append(",");
					}
					SB.Append(NormalizedInputFiles[i]);
				}
				byte[] InvariantBytes = Encoding.Unicode.GetBytes(SB.ToString());
				return BitConverter.ToString(Hasher.ComputeHash(InvariantBytes)).Replace("-", "").ToLowerInvariant().Substring(0, 12);
			}
#else
			return String.Concat(InputFiles.Select(x => x.FullName)).GetHashCode().ToString();
#endif
		}

		public FileItem GetSafeFileItem(FileReference Location)
		{
			if (Location == null)
			{
				throw new InvalidOperationException("GetSafeFileItem got passed a null file reference!");
			}
			return FileItem.GetItemByFileReference(Location);
		}

#if NETCOREAPP
		private FileItem GenerateCompilerCommandsJson(List<IExternalAction> Actions, DirectoryReference OutputDir, List<FileItem> InputFiles, IActionGraphBuilder Graph, ref Dictionary<IExternalAction, List<FileReference>> AdditionalDictionary, ref Dictionary<IExternalAction, FileItem> DependencyListDictionary)
#else
        private FileItem GenerateCompilerCommandsJson(List<Action> Actions, DirectoryReference OutputDir, List<FileItem> InputFiles, IActionGraphBuilder Graph, ref Dictionary<Action, List<FileReference>> AdditionalDictionary, ref Dictionary<Action, FileItem> DependencyListDictionary)
#endif
		{
			List<string> CommandsJsonEntries = new List<string>();

			foreach (var Action in Actions.OrderBy(x => 
			{
				FileItem SourceFileItem = TryGetPrerequisiteSourceFile(x);
				if (SourceFileItem == null)
				{
                    return string.Empty;
                }
                return SourceFileItem.Location.ToString().ToLowerInvariant();
            }))
			{
				if (Action.ActionType != ActionType.Compile)
				{
					continue;
				}

				FileItem SourceFileItem = TryGetPrerequisiteSourceFile(Action);
				if (SourceFileItem == null)
				{
					continue;
				}

				List<FileReference> AdditionalDependencies = new List<FileReference>();
				FileItem OriginalResponseFile = GetPrerequisiteResponseFile(Action);
				FileItem DependencyListFile;
				FileItem ClangResponseFile = GenerateClangResponseFile(OriginalResponseFile, Graph, ref AdditionalDependencies, false, out DependencyListFile);
				AdditionalDictionary[Action] = AdditionalDependencies;
				DependencyListDictionary[Action] = DependencyListFile;

				CommandsJsonEntries.Add(@"
    {
        'directory': '" + GetEngineSourceDirectory() + @"',
        'file': '" + SourceFileItem.Location + @"',
        'command': 'clang-cl.exe /DUNREAL_CODE_ANALYZER=1 @""" + ClangResponseFile.Location + @"""'
    }
");
			}

			String InputFilesHash = ComputeInputFileHash(InputFiles);

			if (ShouldEmitDiagnostics)
			{
				Console.WriteLine("ClangCommandsDir: Calculated commands directory: " + Path.Combine(OutputDir.FullName, ClangVariantSuffix, InputFilesHash));
			}
			DirectoryReference ClangCommandsDir = new DirectoryReference(Path.Combine(OutputDir.FullName, ClangVariantSuffix, InputFilesHash));
			Directory.CreateDirectory(ClangCommandsDir.FullName);

			FileReference CompileCommandsFileName = new FileReference(Path.Combine(ClangCommandsDir.FullName, "compile_commands.json"));
#if UE5_MAIN
            FileItem CompileCommands = Graph.CreateIntermediateTextFile(CompileCommandsFileName, "[" + string.Join(",", CommandsJsonEntries) + "]", StringComparison.Ordinal);
#else
			FileItem CompileCommands = Graph.CreateIntermediateTextFile(CompileCommandsFileName, "[" + string.Join(",", CommandsJsonEntries) + "]");
#endif

			return CompileCommands;
		}

		private FileItem GenerateClangResponseFile(FileItem OriginalResponseFile, IActionGraphBuilder Graph, ref List<FileReference> AdditionalDependencies, bool bSilenceWarnings, out FileItem DependencyListFile)
		{
			String[] ResponseFileLines = File.ReadAllLines(OriginalResponseFile.Location.FullName);
			bool bIsCreatingPCH = false;
			String GeneratedPCHHeader = String.Empty;
			String UsedPCHHeader = String.Empty;
			// Detect bIsCreatingPCH first, because for some invocations the /Yc flag occurs *after* the .cpp file. If we
			// determine bIsCreatingPCH in the main loop, then we won't process the PCH .cpp file because we haven't seen
			// /Yc first.
			for (int i = 0; i < ResponseFileLines.Length; i++)
			{
				if (ResponseFileLines[i].StartsWith("/Yc"))
				{
					String Flag = ResponseFileLines[i].Substring(1, 2);
					FileReference FileName = new FileReference(ResponseFileLines[i].Substring(4, ResponseFileLines[i].Trim().Length - 5));
					FileReference NewFileName = GetClangVariantFilename(FileName);
					if (FileName.HasExtension(".h") && (FileName.GetFileName().StartsWith("PCH.") || FileName.GetFileName().StartsWith("SharedPCH.")))
					{
						if (Flag == "Yc")
						{
							bIsCreatingPCH = true;
							GeneratedPCHHeader = NewFileName.FullName;
							break;
						}
					}
				}
			}
			for (int i = 0; i < ResponseFileLines.Length; i++)
			{
				if (ResponseFileLines[i].StartsWith("/FI") ||
					ResponseFileLines[i].StartsWith("/Yu") ||
					ResponseFileLines[i].StartsWith("/Yc") ||
					ResponseFileLines[i].StartsWith("/Fp") ||
					ResponseFileLines[i].StartsWith("/Fo"))
				{
					// Extract the filename.
					String Flag = ResponseFileLines[i].Substring(1, 2);
					FileReference FileName = new FileReference(ResponseFileLines[i].Substring(4, ResponseFileLines[i].Trim().Length - 5));

					// Generate the new filename.
					FileReference NewFileName = GetClangVariantFilename(FileName);

					// Are we substituting this filename?
					bool bWillSubstitute = false;

					// Process the file based on it's extension.
					if (FileName.HasExtension(".h") && (FileName.GetFileName().StartsWith("PCH.") || FileName.GetFileName().StartsWith("SharedPCH.")))
					{
						// Copy the header file.
						Directory.CreateDirectory(NewFileName.Directory.FullName);
						Graph.CreateCopyAction(FileName, NewFileName);
						AdditionalDependencies.Add(FileName);
						AdditionalDependencies.Add(NewFileName);
						if (Flag == "Yc")
						{
							// This is now handled above.
						}
						else if (Flag == "Yu")
						{
							UsedPCHHeader = NewFileName.FullName;
						}
						bWillSubstitute = true;
					}
					else if (FileName.HasExtension(".pch") ||
							 FileName.HasExtension(".obj"))
					{
						// Nothing to do; we'll generate it this as an output.
						bWillSubstitute = true;
					}

					if (bWillSubstitute)
					{
						if (ShouldEmitDiagnostics)
						{
							Console.WriteLine("Substituting: " + ResponseFileLines[i] + " -> " + String.Format("/{0}\"{1}\"", Flag, NewFileName));
						}
						ResponseFileLines[i] = String.Format("/{0}\"{1}\"", Flag, NewFileName);
					}
				}
				else if (ResponseFileLines[i] == "/W4" && (bSilenceWarnings || bIsCreatingPCH))
				{
					ResponseFileLines[i] = "/W0";
				}
				else if (ResponseFileLines[i] == "/WX")
				{
					// Turn off "warnings as errors". This used to be a UE5-only flag, but you can customize the default
					// warning level per target in older versions of Unreal, which can also cause this flag to be present.
					// When this flag is present, clang-tidy will emit errors for compiler warnings it would not normally
					// emit.
					ResponseFileLines[i] = "";
				}
				else if (ResponseFileLines[i].StartsWith("/we"))
				{
					// Do not enable all "warnings as errors" for Clang.
					ResponseFileLines[i] = "";
				}
				else if (
					(
						ResponseFileLines[i].StartsWith("\"") ||
						(
							ResponseFileLines[i].EndsWith(".cpp") &&
							!ResponseFileLines[i].StartsWith("/") &&
							!ResponseFileLines[i].StartsWith("-")
						)
					) && bIsCreatingPCH)
				{
					// We need to alter the C++ file because it's including the PCH header directly, and
					// we have to use the new path we made.
					var FileNameRaw = ResponseFileLines[i].StartsWith("\"") ? ResponseFileLines[i].Substring(1, ResponseFileLines[i].Trim().Length - 2) : ResponseFileLines[i];
#if UE5_MAIN
                    if (!Path.IsPathRooted(FileNameRaw))
                    {
                        FileNameRaw = Path.Combine(Unreal.EngineDirectory.FullName, "Source", FileNameRaw);
                    }
#endif
					FileReference FileName = new FileReference(FileNameRaw);
					FileReference NewFileName = GetClangVariantFilename(FileName);

#if UE5_MAIN
                    Graph.CreateIntermediateTextFile(NewFileName, "#include \"" + GeneratedPCHHeader + "\"", StringComparison.Ordinal);
#else
					Graph.CreateIntermediateTextFile(NewFileName, "#include \"" + GeneratedPCHHeader + "\"");
#endif
					AdditionalDependencies.Add(new FileReference(GeneratedPCHHeader));
					AdditionalDependencies.Add(FileName);
					AdditionalDependencies.Add(NewFileName);

					ResponseFileLines[i] = "\"" + NewFileName.FullName + "\"";
				}
				else if (ResponseFileLines[i].StartsWith("/fastfail"))
				{
					// Ignore this flag; it does not apply to clang-tidy.
					ResponseFileLines[i] = "";
				}
			}

			// HACK: In some files, it seems we have to duplicate /FI to get the PCH forcibly included under Clang...
			List<String> ResponseFileLinesList = ResponseFileLines.ToList();
			if (UsedPCHHeader != String.Empty)
			{
				int Index = ResponseFileLinesList.IndexOf("/FI\"" + UsedPCHHeader + "\"");
				if (Index != -1)
				{
					ResponseFileLinesList.Insert(Index, "/FI\"" + UsedPCHHeader + "\"");
				}
			}

			FileReference NewResponseFile = GetClangVariantFilename(OriginalResponseFile.Location);
			DependencyListFile = FileItem.GetItemByFileReference(FileReference.Combine(NewResponseFile.Directory, NewResponseFile.GetFileName() + ".d"));
			ResponseFileLinesList = new List<string>
			{
				"/clang:-MD",
				string.Format("/clang:-MF\"{0}\"", DependencyListFile.AbsolutePath),
				"/clang:-Wno-c++11-narrowing"
			}.Concat(ResponseFileLinesList).ToList();

#if UE5_MAIN
            if (ShouldEmitDiagnostics)
            {
                System.Console.WriteLine("Wrote response file to: " + GetClangVariantFilename(OriginalResponseFile.Location));
            }
            return Graph.CreateIntermediateTextFile(GetClangVariantFilename(OriginalResponseFile.Location), String.Join(Environment.NewLine, ResponseFileLinesList), StringComparison.Ordinal);
#else
			return Graph.CreateIntermediateTextFile(GetClangVariantFilename(OriginalResponseFile.Location), String.Join(Environment.NewLine, ResponseFileLinesList));
#endif
		}

        public override CPPOutput CompileCPPFiles(CppCompileEnvironment CompileEnvironment, List<FileItem> InputFiles, DirectoryReference OutputDir, string ModuleName, IActionGraphBuilder Graph)
		{
#if NETCOREAPP
			List<IExternalAction> PreprocessActions = new List<IExternalAction>();
#else
            List<Action> PreprocessActions = new List<Action>();
#endif

#if NETCOREAPP
			// We no longer support SN-DBS in Unreal Engine 5, as our previous workaround
			// no longer works for some reason (and causes duplicate actions to appear in
			// the graph which doubles the number of clang-tidy invocations).
			CPPOutput Result = base.CompileCPPFiles(CompileEnvironment, InputFiles, OutputDir, ModuleName, new ActionGraphCapture(Graph, PreprocessActions));
#else
			// We have to run without bGenerateDependenciesFile so we can get dependencies for
			// generating our clang-tidy commands, but we need bGenerateDependenciesFile to be
			// turned on so the actual ac tions use cl-filter which allows SN-DBS to work.
			CPPOutput Result;
			if (CompileEnvironment.bGenerateDependenciesFile)
			{
				CompileEnvironment.bGenerateDependenciesFile = false;
				base.CompileCPPFiles(CompileEnvironment, InputFiles, OutputDir, ModuleName, new ActionGraphCaptureOnly(Graph, PreprocessActions));
				CompileEnvironment.bGenerateDependenciesFile = true;
				Result = base.CompileCPPFiles(CompileEnvironment, InputFiles, OutputDir, ModuleName, Graph);
			}
			else
			{
				Result = base.CompileCPPFiles(CompileEnvironment, InputFiles, OutputDir, ModuleName, new ActionGraphCapture(Graph, PreprocessActions));
			}
#endif

			// If clang-tidy is not installed, don't run it.
			if (ClangTidyAnalyzerFile == null)
			{
				return Result;
			}

			// If this is a Live Coding build, don't run clang-tidy.
			if (IsLiveCoding)
			{
				return Result;
			}

#if NETCOREAPP
			// Unreal Engine 5 isn't compatible with running the analyzer on Shipping builds (there's
			// code in UE5 that gets activated based on __clang_analyzer__, but relies on references that
			// aren't available in UE_BUILD_SHIPPING, which leads to clang-tidy compilation errors).
			if (CompileEnvironment.Configuration == CppConfiguration.Shipping)
			{
				return Result;
			}
#endif

			// Locate the ClangTidy.lua file that we will use when invoking clang-tidy. The presence of ClangTidy.lua is now
			// what enables clang-tidy on a module, not the header defines.
			string ClangTidyLuaPath = null;
#if NET6_0
			var TargetMakefile = (Graph as TargetMakefileBuilder).Makefile;
#else
			var TargetMakefile = Graph as TargetMakefile;
#endif
			if (TargetMakefile == null)
			{
				Console.WriteLine("warning: Action builder graph was not TargetMakefile (was " + Graph.GetType().FullName + ") - can't enable clang-tidy!");
			}
			else
			{
				DirectoryItem SourceDirectory = TargetMakefile.SourceDirectories.FirstOrDefault(x => x.Name == ModuleName);
				if (SourceDirectory != null)
				{
					var SearchPaths = new List<string>();
					for (var i = 0; i < 6; i++)
					{
						SearchPaths.Add(Path.Combine(SourceDirectory.FullName, "ClangTidy.lua"));
						SearchPaths.Add(Path.Combine(SourceDirectory.FullName, "Resources", "ClangTidy.lua"));
						if (SourceDirectory.EnumerateFiles().Any(x => x.HasExtension(".uproject") || x.HasExtension(".uplugin")))
						{
							break;
						}
						SourceDirectory = SourceDirectory.GetParentDirectoryItem();
						if (SourceDirectory == null)
						{
							break;
						}
					}
					foreach (var SearchPath in SearchPaths)
					{
						if (File.Exists(SearchPath))
						{
							ClangTidyLuaPath = SearchPath;

							// Emit the clang-tidy Lua path if diagnostics are enabled.
							if (ShouldEmitDiagnostics)
							{
								Console.WriteLine("Module '" + ModuleName + "' will use '" + SearchPath + "' file for clang-tidy, based on a source directory of '" + SourceDirectory.FullName + "'. There are " + TargetMakefile.SourceDirectories.Where(x => x.Name == ModuleName).Count() + " source directories for this module.");
							}
						}
					}
				}
			}

			// Only run clang-tidy if it is enabled for this module.
			bool bIsClangTidyEnabled = false;
			if (ModuleName == "Shared")
			{
				// We must always be enabled for the Shared module, as this is the module that generates SharedPCH files. We must always produce the clang-tidy variants of shared PCHs, otherwise game modules will not be able to "build" under clang-tidy properly.
				bIsClangTidyEnabled = true;
			}
			else if (CompileEnvironment.Definitions.Contains(ModuleName + "_ENABLE_CLANG_TIDY=0"))
			{
				bIsClangTidyEnabled = false;
			}
			else
			{
				bool bDidOverride = false;
				foreach (FileItem Item in CompileEnvironment.ForceIncludeFiles)
				{
					string[] Definitions = File.ReadAllLines(Item.AbsolutePath);
					if (Definitions.Contains("#define " + ModuleName + "_ENABLE_CLANG_TIDY 0"))
					{
						bIsClangTidyEnabled = false;
						bDidOverride = true;
					}
				}
				if (!bDidOverride)
				{
					if (ClangTidyLuaPath != null)
					{
						bIsClangTidyEnabled = true;
					}
				}
			}
			if (!bIsClangTidyEnabled)
			{
				return Result;
			}

			// Find any commands that produce PCH files, and re-run them under Clang, mutating the location they emit their PCH from. This is required because the PCH format is different between MSVC and Clang, and clang-tidy will need to load in the Clang PCH format.
			foreach (var PreprocessAction in PreprocessActions)
			{
				if (PreprocessAction.ActionType != ActionType.Compile)
				{
					continue;
				}

				if (ProducesPCHFile(PreprocessAction))
				{
					FileItem OriginalPCHFile = GetProducedPCHFile(PreprocessAction);
					FileItem ClangPCHFile = GetSafeFileItem(GetClangVariantFilename(OriginalPCHFile.Location));

					FileItem OriginalObjFile = GetProducedObjFile(PreprocessAction);
					FileItem ClangObjFile = GetSafeFileItem(GetClangVariantFilename(OriginalObjFile.Location));

					List<FileReference> AdditionalDependencies = new List<FileReference>();
					FileItem OriginalResponseFile = GetPrerequisiteResponseFile(PreprocessAction);
					FileItem DependencyListFile;
					FileItem ClangResponseFile = GenerateClangResponseFile(OriginalResponseFile, Graph, ref AdditionalDependencies, true, out DependencyListFile);

                    Action GeneratePCHAction = Graph.CreateAction(ActionType.Compile);
					GeneratePCHAction.CommandDescription = "Compiling";
					GeneratePCHAction.StatusDescription = PreprocessAction.StatusDescription + " (clang-tidy PCH)";
					GeneratePCHAction.WorkingDirectory = GetEngineSourceDirectory();
					GeneratePCHAction.CommandPath = ClangClCompilerFile;
					GeneratePCHAction.CommandArguments = "/DUNREAL_CODE_ANALYZER=1 @\"" + ClangResponseFile + "\"";
					GeneratePCHAction.PrerequisiteItems.AddRange(PreprocessAction.PrerequisiteItems.Where(x => !IsResponseFile(x.Location)));
					GeneratePCHAction.PrerequisiteItems.AddRange(AdditionalDependencies.Select(x => GetSafeFileItem(x)));
					GeneratePCHAction.PrerequisiteItems.Add(ClangResponseFile);
					GeneratePCHAction.PrerequisiteItems.Add(GetSafeFileItem(ClangClCompilerFile));
					GeneratePCHAction.bCanExecuteRemotely = true;
					// GeneratePCHAction.bCanExecuteRemotelyWithSNDBS = true;
					GeneratePCHAction.ProducedItems.Add(ClangPCHFile);
					GeneratePCHAction.ProducedItems.Add(ClangObjFile);
					GeneratePCHAction.DependencyListFile = DependencyListFile;
					GeneratePCHAction.ProducedItems.Add(DependencyListFile);

					Result.ObjectFiles.AddRange(GeneratePCHAction.ProducedItems);
				}
			}

			// At this point, we if don't have a Lua file, then we aren't actually going to run clang-tidy (we might just have been building PCHs for Shared).
			if (ClangTidyLuaPath == null)
			{
				return Result;
			}

			KnownClangLuaPaths.Add(ClangTidyLuaPath);

#if NETCOREAPP
			Dictionary<IExternalAction, List<FileReference>> AdditionalDictionary = new Dictionary<IExternalAction, List<FileReference>>();
			Dictionary<IExternalAction, FileItem> DependencyListDictionary = new Dictionary<IExternalAction, FileItem>();
#else
            Dictionary<Action, List<FileReference>> AdditionalDictionary = new Dictionary<Action, List<FileReference>>();
            Dictionary<Action, FileItem> DependencyListDictionary = new Dictionary<Action, FileItem>();
#endif
			FileItem CompileCommandsJson = GenerateCompilerCommandsJson(
				PreprocessActions,
				OutputDir,
				InputFiles,
				Graph,
				ref AdditionalDictionary,
				ref DependencyListDictionary);

			bool bShowClangQueryCommands = false;
			if (CompileEnvironment.Definitions.Contains(ModuleName + "_CLANG_TIDY_DISPLAY_QUERY_COMMANDS=1"))
			{
				bShowClangQueryCommands = false;
			}
			else
			{
				foreach (FileItem Item in CompileEnvironment.ForceIncludeFiles)
				{
					string[] Definitions = File.ReadAllLines(Item.AbsolutePath);
					if (Definitions.Contains("#define " + ModuleName + "_CLANG_TIDY_DISPLAY_QUERY_COMMANDS 1"))
					{
						bShowClangQueryCommands = true;
						break;
					}
				}
			}

			foreach (var PreprocessAction in PreprocessActions)
			{
				if (PreprocessAction.ActionType != ActionType.Compile)
				{
					continue;
				}

				FileItem SourceFileItem = TryGetPrerequisiteSourceFile(PreprocessAction);
				if (SourceFileItem == null)
				{
					continue;
				}

				if (IsGeneratedCppFile(SourceFileItem.Location))
				{
					// Don't check the C++ files that UHT generates.
					continue;
				}

				FileReference ClangTouchFileReference = GetClangVariantFilename(new FileReference(Path.Combine(OutputDir.FullName, SourceFileItem.Location.GetFileName() + ".cttouch")));

				if (bShowClangQueryCommands)
				{
					Console.WriteLine("[clang-query] \"{0}\" \"-p={1}\" \"--header-filter=.*/{2}/.*\" \"{3}\"",
						ClangQueryFile.FullName,
						CompileCommandsJson.Location,
						ModuleName,
						SourceFileItem.Location);
				}

				Action AnalyzeAction = Graph.CreateAction(ActionType.Compile);
				AnalyzeAction.CommandDescription = "Analyzing";
#if NETCOREAPP
				// Prevent ParallelExecutor from emitting StatusDescription (clang-tidy.exe already does it). We only
				// want the StatusDescription to be used by e.g. the XGE user interface, and never emitted to the
				// output.
                AnalyzeAction.bShouldOutputStatusDescription = false;
#endif
                AnalyzeAction.StatusDescription = PreprocessAction.StatusDescription + " (clang-tidy)";
				AnalyzeAction.WorkingDirectory = GetEngineSourceDirectory();
				AnalyzeAction.CommandPath = ClangTidyAnalyzerFile;
				AnalyzeAction.CommandArguments = string.Format(
					"--lua-script-path=\"{5}\" --lua-script-path=\"{3}\" --checks=-* --touch-path=\"{4}\" --quiet \"--header-filter=.*/{2}/.*\" \"-p={0}\" \"{1}\"",
					CompileCommandsJson.Location,
					SourceFileItem.Location,
					ModuleName,
					ClangTidyLuaPath,
					ClangTouchFileReference,
					ClangTidySystemLuaFile);
				AnalyzeAction.ProducedItems.Add(GetSafeFileItem(ClangTouchFileReference));
				AnalyzeAction.PrerequisiteItems.Add(CompileCommandsJson);
				AnalyzeAction.PrerequisiteItems.Add(GetSafeFileItem(ClangTidyAnalyzerFile));
				AnalyzeAction.PrerequisiteItems.Add(GetSafeFileItem(ClangTidySystemLuaFile));
				AnalyzeAction.PrerequisiteItems.Add(GetSafeFileItem(new FileReference(ClangTidyLuaPath)));
				AnalyzeAction.bCanExecuteRemotely = true;
				// AnalyzeAction.bCanExecuteRemotelyWithSNDBS = true;
				AnalyzeAction.PrerequisiteItems.AddRange(
					PreprocessAction.PrerequisiteItems
						.Where(x => IsResponseFile(x.Location) || IsPCHFile(x.Location))
						.Select(x => GetSafeFileItem(GetClangVariantFilename(x.Location))));
				AnalyzeAction.PrerequisiteItems.AddRange(
					PreprocessAction.PrerequisiteItems
						.Where(x => IsSourceFile(x.Location))
						.Select(x => GetSafeFileItem(x.Location)));
				AnalyzeAction.DependencyListFile = DependencyListDictionary[PreprocessAction];
				AnalyzeAction.ProducedItems.Add(DependencyListDictionary[PreprocessAction]);

				Result.ObjectFiles.AddRange(AnalyzeAction.ProducedItems);
			}

			return Result;
		}

		public override void GetExternalDependencies(HashSet<FileItem> ExternalDependencies)
		{
			base.GetExternalDependencies(ExternalDependencies);

			foreach (var FilePath in this.KnownClangLuaPaths)
			{
				ExternalDependencies.Add(GetSafeFileItem(new FileReference(FilePath)));
			}
		}

		public override FileItem LinkFiles(LinkEnvironment LinkEnvironment, bool bBuildImportLibraryOnly, IActionGraphBuilder Graph)
		{
			// If this is a Live Coding build, don't run clang-tidy.
			if (IsLiveCoding)
			{
				return base.LinkFiles(LinkEnvironment, bBuildImportLibraryOnly, Graph);
			}

			// Seperate our Clang files from our link stage.
			List<FileItem> ClangFiles = LinkEnvironment.InputFiles.Where(x => IsClangVariantFilename(x.Location)).ToList();
			LinkEnvironment.InputFiles = LinkEnvironment.InputFiles.Where(x => !IsClangVariantFilename(x.Location)).ToList();

			// Generate the link action using VCToolChain.
#if NETCOREAPP
			List<IExternalAction> LinkActions = new List<IExternalAction>();
#else
            List<Action> LinkActions = new List<Action>();
#endif
			FileItem LinkResult = base.LinkFiles(LinkEnvironment, bBuildImportLibraryOnly, new ActionGraphCapture(Graph, LinkActions));

			// Now attach our Clang files as inputs to the link action, even though they won't be used as arguments to the command. This ensures that the build graph runs our clang-tidy related actions.
			((Action)LinkActions[0]).PrerequisiteItems.AddRange(ClangFiles);

			return LinkResult;
		}
	}
}

