################################################################################
### CONSTANTS ##################################################################
################################################################################

# URL of the LabelIMG download
#     ! SHOULD ONLY BE ALTERED IF A NEW VERSION IS AVAILABLE !
$URL = "https://www.dropbox.com/s/4cx5i1rq4w2079s/windows_v1.6.1.zip?dl=1"

# Path where LabelIMG should be installed to
#     May be altered.
$TARGETDIR = "$PSScriptRoot/LabelIMG"

# Path and file name where the LabelIMG.zip should be placed
#     ! SHOULD NOT BE ALTERED !
$TARGETFILEZIP = "$TARGETDIR/labelIMG.zip"

# Path and file name where the LabelIMG.exe is located after dezipping.
#     ! SHOULD NOT BE ALTERED !
$FILEEXE = "$TARGETDIR/windows_v1.6.1/labelImg.exe"

# Path and file name where the classes list should be placed
#     ! SHOULD NOT BE ALTERED !
$TARGETFILECONFIG = "$TARGETDIR/windows_v1.6.1/data/predefined_classes.txt"

################################################################################

# Creates a new folder for LabelIMG, if it doesn't exist already
if(!(Test-Path -Path $TARGETDIR )){
    New-Item -ItemType directory -Path $TARGETDIR
}

# Downloads the LabelIMG.zip to the targeted location if it isn't placed there
#     already.
if(!(Test-Path $TARGETFILEZIP)){
    Invoke-WebRequest -Uri $URL -OutFile $TARGETFILEZIP
}

# Unzips the LabelIMG.zip if it isn't already.
if(!(Test-Path $FILEEXE)){
    Expand-Archive -path $TARGETFILEZIP -DestinationPath $TARGETDIR
}

# Copies the provided class-list to its correct location
Copy-Item "predefined_classes.txt" -Destination $TARGETFILECONFIG