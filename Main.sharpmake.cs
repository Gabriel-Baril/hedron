using System.IO;
using Sharpmake;

public static class Constants
{
    public const string DEFAULT_SOURCE_FOLDER = "src";
    public const string MAIN_SHARPMAKE_FILE = "main.sharpmake.cs";

    public const string EXTERNAL_VS_CATEGORY = "external";
    public const string MODULE_VS_CATEGORY = "module";
    public const string PLUGIN_VS_CATEGORY = "plugin";
    public const string EXPERIMENTAL_VS_CATEGORY = "experimental";

    public const string MODULE_FOLDER = "module";
    public static string[] SHARPMAKE_FOLDERS = { Constants.MODULE_FOLDER };
}

public static class Paths
{
    public static readonly string MainFileDirectory = Directory.GetCurrentDirectory(); // Points to the home of the Main.sharpmake.cs
}

public static class TargetUtil
{
    public static readonly Target DefaultTarget = new Target(Platform.win32 | Platform.win64, DevEnv.vs2022, Optimization.Debug | Optimization.Release | Optimization.Retail);
}

public abstract class BaseCppProject : Project
{
    public BaseCppProject()
    {
        IsFileNameToLower = false;
        IsTargetFileNameToLower = false;
    }

    public void ConfigureAll(Project.Configuration conf, Target target)
    {
        conf.AdditionalCompilerOptions.Add("/utf-8");
        conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings("4201"));

        switch(target.Optimization)
        {
            case Optimization.Debug:
                conf.Defines.Add("_HDN_DEBUG");
                break;
            case Optimization.Release:
                conf.Defines.Add("_HDN_RELEASE");
                break;
            case Optimization.Retail:
                conf.Defines.Add("_HDN_RETAIL");
                break;
        }
    }
}

public static class Main
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Sharpmake.Arguments args)
    {
        args.Generate<IdaesSolution>();
        args.Generate<IdeonSolution>();
        args.Generate<PolySolution>();
        args.Generate<PlaygroundSolution>();
        args.Generate<AllSharpmakeSolution>();
    }
}
