

#include <windows.h>
#include <stdio.h>
#include <iostream>

#include <string>

#include <Bitmap.h>

#include <NvFBCLibrary.h>
#include <NvFBC/nvFBCToSys.h>


// Structure to store the command line arguments
typedef struct _AppArguments
{
    int   iFrameCnt; // Number of frames to grab
    int   iStartX; // Grab start x coord
    int   iStartY; // Grab start y coord
    int   iWidth; // Grab width
    int   iHeight; // Grab height
    int   iSetUpFlags; // Set up flags
    bool  bHWCursor; // Grab hardware cursor
    NVFBCToSysGrabMode     eGrabMode; // Grab mode
    NVFBCToSysBufferFormat eBufFormat; // Output buffer format
    std::string sBaseName; // Output file name
    _AppArguments()
        : iFrameCnt(0)
        , iStartX(0)
        , iStartY(0)
        , iWidth(0)
        , iHeight(0)
        , iSetUpFlags(0)
        , bHWCursor(false)
        , eGrabMode(NVFBC_TOSYS_SOURCEMODE_FULL)
        , eBufFormat(NVFBC_TOSYS_ARGB)
    {
        sBaseName.clear();
    }
} AppArguments;

// Prints the help message
void printHelp()
{
    printf("Usage: NvFBCToSys [options]\n");
    printf("  -frames frameCnt     The number of frames to grab\n");
    printf("                       , this value defaults to one.  If\n");
    printf("                       the value is greater than one only\n");
    printf("                       the final frame is saved as a bitmap.\n");
    printf("  -scale width height  Scales the grabbed frame buffer\n");
    printf("                       to the provided width and height\n");
    printf("  -crop x y width height  Crops the grabbed frame buffer to\n");
    printf("                          the provided region\n");
    printf("  -grabCursor          Grabs the hardware cursor\n");
    printf("  -format <ARGB|RGB|YUV420|YUV444|PLANAR|XOR|ARGB10>\n");
    printf("                       Sets the grab format\n");
    printf("  -output              Output file name\n"
        "                       supports bmp output only\n");
    printf("  -nowait              Grab with the no wait flag\n");
}

// Parse the command line arguments
bool parseCmdLine(int argc, char** argv, AppArguments& args)
{
    args.iFrameCnt = 1;
    args.iStartX = 0;
    args.iStartY = 0;
    args.iWidth = 0;
    args.iHeight = 0;
    args.iSetUpFlags = NVFBC_TOSYS_NOFLAGS;
    args.bHWCursor = false;
    args.eGrabMode = NVFBC_TOSYS_SOURCEMODE_FULL;
    args.eBufFormat = NVFBC_TOSYS_ARGB;
    args.sBaseName = "NvFBCToSys";

    for (int cnt = 1; cnt < argc; ++cnt)
    {
        if (0 == _stricmp(argv[cnt], "-frames"))
        {
            ++cnt;

            if (cnt >= argc)
            {
                printf("Missing -frames option\n");
                printHelp();
                return false;
            }

            args.iFrameCnt = atoi(argv[cnt]);

            // Must grab at least one frame.
            if (args.iFrameCnt < 1)
                args.iFrameCnt = 1;
        }
        else if (0 == _stricmp(argv[cnt], "-scale"))
        {
            if (NVFBC_TOSYS_SOURCEMODE_FULL != args.eGrabMode)
            {
                printf("Both -crop and -scale cannot be used at the same time.\n");
                printHelp();
                return false;
            }

            if ((cnt + 2) > argc)
            {
                printf("Missing -scale options\n");
                printHelp();
                return false;
            }

            args.iWidth = atoi(argv[cnt + 1]);
            args.iHeight = atoi(argv[cnt + 2]);
            args.eGrabMode = NVFBC_TOSYS_SOURCEMODE_SCALE;

            cnt += 2;
        }
        else if (0 == _stricmp(argv[cnt], "-crop"))
        {
            if (NVFBC_TOSYS_SOURCEMODE_FULL != args.eGrabMode)
            {
                printf("Both -crop and -scale cannot be used at the same time.\n");
                printHelp();
                return false;
            }

            if ((cnt + 4) >= argc)
            {
                printf("Missing -crop options\n");
                printHelp();
                return false;
            }

            args.iStartX = atoi(argv[cnt + 1]);
            args.iStartY = atoi(argv[cnt + 2]);
            args.iWidth = atoi(argv[cnt + 3]);
            args.iHeight = atoi(argv[cnt + 4]);
            args.eGrabMode = NVFBC_TOSYS_SOURCEMODE_CROP;

            cnt += 4;
        }
        else if (0 == _stricmp(argv[cnt], "-grabCursor"))
        {
            args.bHWCursor = true;
        }
        else if (0 == _stricmp(argv[cnt], "-nowait"))
        {
            args.iSetUpFlags |= NVFBC_TOSYS_NOWAIT;
        }
        else if (0 == _stricmp(argv[cnt], "-format"))
        {
            ++cnt;

            if (cnt >= argc)
            {
                printf("Missing -format option\n");
                printHelp();
                return false;
            }

            if (0 == _stricmp(argv[cnt], "ARGB"))
                args.eBufFormat = NVFBC_TOSYS_ARGB;
            else if (0 == _stricmp(argv[cnt], "RGB"))
                args.eBufFormat = NVFBC_TOSYS_RGB;
            else if (0 == _stricmp(argv[cnt], "YUV420"))
                args.eBufFormat = NVFBC_TOSYS_YYYYUV420p;
            else if (0 == _stricmp(argv[cnt], "YUV444"))
                args.eBufFormat = NVFBC_TOSYS_YUV444p;
            else if (0 == _stricmp(argv[cnt], "PLANAR"))
                args.eBufFormat = NVFBC_TOSYS_RGB_PLANAR;
            else if (0 == _stricmp(argv[cnt], "XOR"))
                args.eBufFormat = NVFBC_TOSYS_XOR;
            else if (0 == _stricmp(argv[cnt], "ARGB10"))
                args.eBufFormat = NVFBC_TOSYS_ARGB10;
            else
            {
                printf("Unexpected -format option %s\n", argv[cnt]);
                printHelp();
                return false;
            }
        }
        else if (0 == _stricmp(argv[cnt], "-output"))
        {
            ++cnt;

            if (cnt >= argc)
            {
                printf("Missing -output option\n");
                printHelp();
                return false;
            }

            args.sBaseName = argv[cnt];
            if (std::string::npos == args.sBaseName.find(".bmp")) {
                args.sBaseName += ".bmp";
            }
        }
        else
        {
            printf("Unexpected argument %s\n", argv[cnt]);
            printHelp();
            return false;
        }
    }

    return true;
}
/*!
 * Main program
 */
int main(int argc, char* argv[])
{
    AppArguments args;

    NvFBCLibrary nvfbcLibrary;
    NvFBCToSys* nvfbcToSys = NULL;

    DWORD maxDisplayWidth = -1, maxDisplayHeight = -1;
    BOOL bRecoveryDone = FALSE;

    NvFBCFrameGrabInfo grabInfo;
    unsigned char* frameBuffer = NULL;
    unsigned char* diffMap = NULL;
    char frameNo[10];
    std::string outName;

    if (!parseCmdLine(argc, argv, args))
        return -1;

    //! Load NvFBC
    if (!nvfbcLibrary.load())
    {
        fprintf(stderr, "Unable to load the NvFBC library\n");
        return -1;
    }

    //! Create an instance of NvFBCToSys
    nvfbcToSys = (NvFBCToSys*)nvfbcLibrary.create(NVFBC_TO_SYS, &maxDisplayWidth, &maxDisplayHeight);

    NVFBCRESULT status = NVFBC_SUCCESS;
    if (!nvfbcToSys)
    {
        fprintf(stderr, "Unable to create an instance of NvFBC\n");
        return -1;
    }

    //! Setup the frame grab
    NVFBC_TOSYS_SETUP_PARAMS fbcSysSetupParams = { 0 };
    fbcSysSetupParams.dwVersion = NVFBC_TOSYS_SETUP_PARAMS_VER;
    fbcSysSetupParams.eMode = args.eBufFormat;
    fbcSysSetupParams.bWithHWCursor = args.bHWCursor;
    fbcSysSetupParams.bDiffMap = FALSE;
    fbcSysSetupParams.ppBuffer = (void**)&frameBuffer;
    fbcSysSetupParams.ppDiffMap = NULL;

    status = nvfbcToSys->NvFBCToSysSetUp(&fbcSysSetupParams);
    if (status == NVFBC_SUCCESS)
    {
        //! Sleep so that ToSysSetUp forces a framebuffer update
        Sleep(100);

        NVFBC_TOSYS_GRAB_FRAME_PARAMS fbcSysGrabParams = { 0 };
        //! For each frame to grab..
        for (int cnt = 0; cnt < args.iFrameCnt; ++cnt)
        {
            outName = args.sBaseName + "_" + _itoa(cnt, frameNo, 10) + ".bmp";
            //! Grab the frame.  
            // If scaling or cropping is enabled the width and height returned in the
            // NvFBCFrameGrabInfo structure reflect the current desktop resolution, not the actual grabbed size.
            fbcSysGrabParams.dwVersion = NVFBC_TOSYS_GRAB_FRAME_PARAMS_VER;
            fbcSysGrabParams.dwFlags = args.iSetUpFlags;
            fbcSysGrabParams.dwTargetWidth = args.iWidth;
            fbcSysGrabParams.dwTargetHeight = args.iHeight;
            fbcSysGrabParams.dwStartX = args.iStartX;
            fbcSysGrabParams.dwStartY = args.iStartY;
            fbcSysGrabParams.eGMode = args.eGrabMode;
            fbcSysGrabParams.pNvFBCFrameGrabInfo = &grabInfo;

            status = nvfbcToSys->NvFBCToSysGrabFrame(&fbcSysGrabParams);
            if (status == NVFBC_SUCCESS)
            {
                bRecoveryDone = FALSE;
                //! Save the frame to disk
                switch (args.eBufFormat)
                {
                case NVFBC_TOSYS_ARGB:
                    SaveARGB(outName.c_str(), frameBuffer, grabInfo.dwWidth, grabInfo.dwHeight, grabInfo.dwBufferWidth);
                    fprintf(stderr, "Grab succeeded. Wrote %s as ARGB.\n", outName.c_str());
                    break;

                case NVFBC_TOSYS_RGB:
                    SaveRGB(outName.c_str(), frameBuffer, grabInfo.dwWidth, grabInfo.dwHeight, grabInfo.dwBufferWidth);
                    fprintf(stderr, "Grab succeeded. Wrote %s as RGB.\n", outName.c_str());
                    break;

                case NVFBC_TOSYS_YUV444p:
                    SaveYUV444(outName.c_str(), frameBuffer, grabInfo.dwWidth, grabInfo.dwHeight);
                    fprintf(stderr, "Grab succeeded. Wrote %s as YUV444 converted to RGB.\n", outName.c_str());
                    break;

                case NVFBC_TOSYS_YYYYUV420p:
                    SaveYUV420(outName.c_str(), frameBuffer, grabInfo.dwWidth, grabInfo.dwHeight);
                    fprintf(stderr, "Grab succeeded. Wrote %s as YYYYUV420p.\n", outName.c_str());
                    break;

                case NVFBC_TOSYS_RGB_PLANAR:
                    SaveRGBPlanar(outName.c_str(), frameBuffer, grabInfo.dwWidth, grabInfo.dwHeight);
                    fprintf(stderr, "Grab succeeded. Wrote %s as RGB_PLANAR.\n", outName.c_str());
                    break;

                case NVFBC_TOSYS_XOR:
                    // The second grab results in the XOR of the first and second frame.
                    fbcSysGrabParams.dwVersion = NVFBC_TOSYS_GRAB_FRAME_PARAMS_VER;
                    fbcSysGrabParams.dwFlags = args.iSetUpFlags;
                    fbcSysGrabParams.dwTargetWidth = args.iWidth;
                    fbcSysGrabParams.dwTargetHeight = args.iHeight;
                    fbcSysGrabParams.dwStartX = 0;
                    fbcSysGrabParams.dwStartY = 0;
                    fbcSysGrabParams.eGMode = args.eGrabMode;
                    fbcSysGrabParams.pNvFBCFrameGrabInfo = &grabInfo;
                    status = nvfbcToSys->NvFBCToSysGrabFrame(&fbcSysGrabParams);
                    if (status == NVFBC_SUCCESS)
                        SaveRGB(outName.c_str(), frameBuffer, grabInfo.dwWidth, grabInfo.dwHeight, grabInfo.dwBufferWidth);

                    fprintf(stderr, "Grab succeeded. Wrote %s as XOR.\n", outName.c_str());
                    break;
                case NVFBC_TOSYS_ARGB10:
                    SaveARGB10(outName.c_str(), frameBuffer, grabInfo.dwWidth, grabInfo.dwHeight, grabInfo.dwBufferWidth);
                    fprintf(stderr, "Grab succeeded. Wrote %s as ARGB10.\n", outName.c_str());
                    break;

                default:
                    fprintf(stderr, "Un-expected grab format %d.", args.eBufFormat);
                    break;
                }
            }
            else
            {
                if (bRecoveryDone == TRUE)
                {
                    fprintf(stderr, "Unable to recover from NvFBC Frame grab failure.\n");
                    goto quit;
                }

                if (status == NVFBC_ERROR_DYNAMIC_DISABLE)
                {
                    fprintf(stderr, "NvFBC disabled. Quitting\n");
                    goto quit;
                }

                if (status == NVFBC_ERROR_INVALIDATED_SESSION)
                {
                    fprintf(stderr, "Session Invalidated. Attempting recovery\n");
                    nvfbcToSys->NvFBCToSysRelease();
                    nvfbcToSys = NULL;
                    //! Recover from error. Create an instance of NvFBCToSys
                    nvfbcToSys = (NvFBCToSys*)nvfbcLibrary.create(NVFBC_TO_SYS, &maxDisplayWidth, &maxDisplayHeight);
                    if (!nvfbcToSys)
                    {
                        fprintf(stderr, "Unable to create an instance of NvFBC\n");
                        goto quit;
                    }
                    //! Setup the frame grab
                    NVFBC_TOSYS_SETUP_PARAMS fbcSysSetupParams = { 0 };
                    fbcSysSetupParams.dwVersion = NVFBC_TOSYS_SETUP_PARAMS_VER;
                    fbcSysSetupParams.eMode = args.eBufFormat;
                    fbcSysSetupParams.bWithHWCursor = args.bHWCursor;
                    fbcSysSetupParams.bDiffMap = FALSE;
                    fbcSysSetupParams.ppBuffer = (void**)&frameBuffer;
                    fbcSysSetupParams.ppDiffMap = NULL;
                    status = nvfbcToSys->NvFBCToSysSetUp(&fbcSysSetupParams);
                    cnt--;
                    if (status == NVFBC_SUCCESS)
                    {
                        bRecoveryDone = TRUE;
                    }
                    else
                    {
                        fprintf(stderr, "Unable to recover from NvFBC Frame grab failure.\n");
                        goto quit;
                    }
                }
            }
        }
    }
    if (status != NVFBC_SUCCESS)
    {
        fprintf(stderr, "Unable to setup frame grab.\n");
    }

quit:
    if (nvfbcToSys)
    {
        //! Relase the NvFBCToSys object
        nvfbcToSys->NvFBCToSysRelease();
    }

    return 0;
}
