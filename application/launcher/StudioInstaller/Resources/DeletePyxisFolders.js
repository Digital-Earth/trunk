var wshShell = new ActiveXObject("WScript.Shell");
var wshEnv = wshShell.Environment("PROCESS");
var pyxisRoamingPath = wshEnv("AppData") + "\\PYXIS\\";
var pyxisLocalPath = wshEnv("AppData") + "\\..\\Local\\PYXIS\\";

var foldersToDelete = new Array(pyxisLocalPath + "BlobCache");

var fso = new ActiveXObject("Scripting.FileSystemObject");

if (fso.FolderExists(pyxisLocalPath)) {

    var folder = fso.GetFolder(pyxisLocalPath);

    for (var subFolder in folder.SubFolders) {
        if (subFolder.Name.toLocaleLowerCase().search("worldviewstudio") != -1) {
            foldersToDelete.push(subFolder.path);
        }
    }

    for (var key in foldersToDelete) {
        try {
            print(foldersToDelete[key]);
            fso.DeleteFolder(foldersToDelete[key]);
        }
        catch (e) { }
    }
}