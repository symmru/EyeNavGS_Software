# 👁️NavGS: A 6-DoF Navigation Dataset and Record-n-Replay Software for Real-World 3DGS Scenes in VR

Zihao Ding¹, Cheng‑Tse Lee², Mufeng Zhu¹, Tao Guan¹, Yuan‑Chun Sun², Cheng‑Hsin Hsu², Yao Liu¹  
¹ Rutgers University | ² National Tsing Hua University  
| [Webpage](https://symmru.github.io/EyeNavGS/) | [Full Paper](https://doi.org/10.48550/arXiv.2506.02380)

---

Abstract: *3D Gaussian Splatting (3DGS) is an emerging media representation that reconstructs real-world 3D scenes in high fidelity, enabling 6-degrees-of-freedom (6-DoF) navigation in virtual reality (VR). However, developing and evaluating 3DGS-enabled applications and optimizing their rendering performance, require realistic user navigation data. Such data is currently unavailable for photorealistic 3DGS reconstructions of real-world scenes. This paper introduces 👁️NavGS (EyeNavGS), the first publicly available 6-DoF navigation dataset featuring traces from 46 participants exploring twelve diverse, real-world 3DGS scenes. The dataset was collected at two sites, using the Meta Quest Pro headsets, recording the head pose and eye gaze data for each rendered frame during free world standing 6-DoF navigation. For each of the twelve scenes, we performed careful scene initialization to correct for scene tilt and scale, ensuring a perceptually-comfortable VR experience. We also release our open-source SIBR viewer software fork with record-and-replay functionalities and a suite of utility tools for data processing, conversion, and visualization. The 👁️NavGS dataset and its accompanying software tools provide valuable resources for advancing research in 6-DoF viewport prediction, adaptive streaming, 3D saliency, and foveated rendering for 3DGS scenes. The 👁️NavGS dataset is available at: [👁️NavGS: 6-DoF Navigation Dataset for 3DGS in VR](https://symmru.github.io/EyeNavGS/).*

<section class="section" id="BibTeX">
  <div class="container is-max-desktop content">
    <h2 class="title">BibTeX</h2>
    <pre><code>
@article{ding2025eyenavgs,
  title={EyeNavGS: A 6-DoF Navigation Dataset and Record-n-Replay Software for Real-World 3DGS Scenes in VR},
  author={Ding, Zihao and Lee, Cheng-Tse and Zhu, Mufeng and Guan, Tao and Sun, Yuan-Chun and Hsu, Cheng-Hsin and Liu, Yao},
  journal={arXiv preprint arXiv:2506.02380},
  year={2025}
}
    </code></pre>
  </div>
</section>

---

## Overview

The **👁️NavGS (EyeNavGS) software** is an open-source tool built on the SIBR viewer, designed for **recording and replaying user navigation in 3D Gaussian Splatting (3DGS) scenes in VR (OpenXR)**. It directly supports the 👁️NavGS dataset collection and offers:

- **Real-time VR viewing** (Desktop and Headset modes).
- **Per-frame trace recording** of head pose, field-of-view, and eye gaze.
- **Offline session replay** to reproduce captured VR experiences and generate stereoscopic video.
- **Utility tools** for coordinate conversions, format interoperability (CSV ↔ JSON), and eye-gaze visualization.

This software lets you visualize, record, and replay 6-DoF user navigation sessions within high-fidelity, photorealistic 3DGS environments.

---

## Installation

Before proceeding, ensure you have a **Windows 10** or **Windows 11** machine (Linux support for Headset Mode via SteamVR is also available). 
You'll need the following:

- **Visual Studio 2019** (with C++20 support)

- **CMake 3.16+**

- **Python 3.8+**

- **Doxygen 1.8.17+**

- **CUDA 10.1+**

- An **NVIDIA GPU**
1. **Clone the repository**:
   
   ```bash
   git clone https://github.com/symmru/EyeNavGS_Software.git
   ```

2. **Verify requirements are in your PATH**:
   
   ```bash
   python --version
   doxygen --version
   nvcc --version
   cmake --version
   ```

3. **Generate Visual Studio project with CMake-GUI**:
   
   - Open CMake GUI.
   
   - Set **“Where is the source code”** to the repository root.
   
   - Set **“Where to build the binaries”** to `<repo_root>/build`.
     ![cmake-1](./docs/img/cmake-1.png)

   - **Fix Compatibility:** Click **Add Entry**, set Name to `CMAKE_POLICY_VERSION_MINIMUM`, Type to `STRING`, and Value to `3.5`. (This prevents errors with legacy libraries like `xatlas`).
   
   - Click **Configure**, choose **Visual Studio 2019 Win64**, then click **Generate**.
     ![cmake-2](./docs/img/cmake-2.png)
   
   - Ensure the following options are enabled:
     
     - `BUILD_SIBR_OPENXR`: ON
     
     - `BUILD_gaussian_viewer`: ON

4. **Compile using Visual Studio**:
   
   - Open `build\sibr_projects.sln` in Visual Studio.
   
   - Select the `ALL_BUILD` target and build (C++ 20).
   
   - Then build the `INSTALL` target.
   
   - The executables (e.g., `SIBR_gaussianViewer_app_d.exe`) will be placed in `install/bin`.
   
   - Add `install/bin` to your PATH for convenience.

---

## Getting Started: The SIBR VR Viewer

The SIBR VR Viewer allows users to visualize 3D Gaussian Splatting scenes either in a Desktop (non-VR) environment or through a VR headset using OpenXR.

For more detailed documentation on the SIBR core system, please refer to the original [SIBR Core](#sibr-core) section and documentation.

The SIBR VR viewer supports two primary modes:

### Desktop Mode

- **Function**: Monocular 2D viewer on PC monitor (no headset).

- **Use Case**: Quickly inspect 3DGS scenes, save camera paths, and record traces without VR hardware.

- **Launch Command**:
  
  ```bash
  SIBR_gaussianViewer_app_d.exe -m <path_to_3DGS_scene> --rendering-mode 0
  ```

- **Interaction**:
  
  - Mouse: rotate, pan, and zoom
  
  - Keyboard (WASD): move camera
  
  ![DesktopMode](./docs/img/Monocular_desktop.png) 

### Headset Mode

- **Function**: Stereoscopic VR experience via [**OpenXR**]([Khronos OpenXR Registry - The Khronos Group Inc](https://registry.khronos.org/OpenXR/)). Supports Meta Quest Pro, Quest 2/3, HTC Vive Pro, etc.

- **Use Case**: Immersive navigation and real-time trace collection.

- **Prerequisites** (for Meta Quest headsets):
  
  1. Install Meta Quest Link (Oculus Link) or SteamVR for Linux.
     
     - [Meta Quest Link](https://www.meta.com/help/quest/1517439565442928/?locale=ko_KR&srsltid=AfmBOoqzLq9v_Gl9v9icmoA9LZYYNThy0XrJzQ6OFVJWrvcZJNL3rWtT) – enable Developer Mode and Eye Tracking permission 
     
     ![MetaLink](./docs/img/quest_link_setting.png)
  
  2. (If eye-tracking available) Calibrate headset’s eye-tracking before recording.

- **Launch Command (no initial settings)**:
  
  ```bash
  SIBR_gaussianViewer_app_d.exe -m <path_to_3DGS_scene> --rendering-mode 2
  ```

- **Launch Command (with scene initialization)**:
  
  ```bash
  SIBR_gaussianViewer_app_d.exe \
    -m <path_to_3DGS_scene> \
    --rendering-mode 2 \
    --initial-position <X> <Y> <Z> \
    --initial-quaternion <QX> <QY> <QZ> <QW> \
    --initial-scale <S>
  ```
  
  - **`--initial-position <X Y Z>`**: Headset origin in world units.
  
  - **`--initial-quaternion <QX QY QZ QW>`**: Scene tilt correction.
  
  - **`--initial-scale <S>`**: Scale of the scene

- Command Example:
  
  ```bash
  SIBR_gaussianViewer_app_d.exe -m D:\projects\sibr\scenes\models\truck --rendering-mode 2 --initial-position 0 2.1 -4 --initial-quaternion  -0.0896, 0.0000, 0.0000, 0.9960 --initial-scale 0.76
  ```
  
  ![Headset](./docs/img/Headset_mode.png)

- **Controller Mapping**: (OpenXR)
  
  | Function                                | Controller Input                            |
  | --------------------------------------- | ------------------------------------------- |
  | Elevate / lower camera position         | Left controller’s vertical stick (↑ / ↓)    |
  | Strafe camera position left / right     | Right controller’s horizontal stick (← / →) |
  | Move camera position forward / backward | Right controller’s vertical stick (↑ / ↓)   |
  | *Rotate scene (around user)             | Right trigger + stick drag                  |
  | *Move scene (drag translation)          | Left trigger + stick drag                   |
  
  > *Note*: 
  > Some commands are also available via the in-app UI (`OpenXR > Configuration`).
  > 
  > Movement speed in the scene can be adjust with the dedicated slider.
  > Press “Save VR configuration” to store `vr.json`, which auto-loads on subsequent launches.

---

## Trace Recording

EyeNavGS captures **per-frame**, per-eye data:

- **ViewIndex (0:left, 1:right)**

- **FOV1–FOV4**: left/right/top/bottom field of view (radians)

- **PositionX, PositionY, PositionZ**: head position (world coordinates)

- **QuaternionX, QuaternionY, QuaternionZ, QuaternionW**: head orientation

- **GazeQX–GazeQW**: eye-gaze orientation quaternion

- **GazePosX, GazePosY, GazePosZ**: eye-gaze position (world coordinates)

- **elapsed_ms**: milliseconds since the `record` button click

- **iso_time**: [**ISO-8601**]([ISO 8601 - Wikipedia](https://en.wikipedia.org/wiki/ISO_8601)) format time in local timezone

### Desktop Mode Recording

1. Launch viewer in Desktop Mode (`--rendering-mode 0`).

2. In the viewer window, click **Record** → **Stop** → **Save path**.

3. The resulting CSV (e.g., `output0.csv`) appears in the chosen folder.
   
   ![openxr gaussian viewer](./docs/img/Saving_trace_Desktop.png)

### Headset Mode Recording

1. Launch viewer in Headset Mode (`--rendering-mode 2`).

2. In the desktop companion window, press **Save Traces** → **Stop Saving**.

3. Each eye’s trace is saved as `output<number>.csv` in the working directory.
   
   * To quickly run different scenes in VR mode on `Windows`, use `utils\WinSceneSelector\scene_select.bat` to choose your desired scene. Once the viewer launches in Headset Mode (`--rendering-mode 2`), press **Start Trace Recording** then **End Trace Recording** in the OpenXR tab in the SIBR viewer to save user's trace.
     
     ![openxr gaussian viewer](./docs/img/HeadSet-trace-save.png)

---

## Trace Format

Each CSV row corresponds to one rendered frame (left eye then right eye). Example columns:

| ViewIndex | FOV1 (rad) | FOV2 (rad) | FOV3 (rad) | FOV4 (rad) | PosX   | PosY   | PosZ  | QuatX | QuatY | QuatZ | QuatW | GazeQX | GazeQY | GazeQZ | GazeQW | GazePosX | GazePosY | GazePosZ | elapsed_ms | iso_time                      |
| --------- | ---------- | ---------- | ---------- | ---------- | ------ | ------ | ----- | ----- | ----- | ----- | ----- | ------ | ------ | ------ | ------ | -------- | -------- | -------- | ---------- | ----------------------------- |
| 0         | -0.9424    | 0.6981     | -0.9424    | 0.7330     | -3.669 | -3.657 | 4.657 | 0.494 | 0.294 | 0.123 | 0.808 | 0.250  | 0.085  | 0.023  | 0.964  | -3.668   | -3.656   | 4.657    | 4.505      | 2025-05-20T16:06:35.004-05:00 |
| 1         | -0.6981    | 0.9424     | -0.9424    | 0.7330     | -3.513 | -3.561 | 4.588 | 0.494 | 0.294 | 0.123 | 0.808 | 0.245  | 0.104  | 0.045  | 0.963  | -3.513   | -3.561   | 4.589    | 8.251      | 2025-05-20T16:06:35.008-05:00 |

> *Without Eye Tracking*: “GazeQX–GazePosZ” fields are all zeros or default (0, 0, 0,1).

---

## Trace Replay

Replay imports a recorded CSV and overrides HMD pose data during rendering.

### Desktop Mode Replay

```bash
SIBR_gaussianViewer_app_d.exe \
  -m <path_to_3DGS_scene> \
  -in <trace_file.csv> \
  --rendering-mode 4 \
  --rendering-size <width> <height>
```

- Default size: 1200 × 789 (per eye).

### Headset Mode Replay

```bash
# Without reinitialization
SIBR_gaussianViewer_app_d.exe \
  -m <path_to_3DGS_scene> \
  -in <output0.csv> \
  --rendering-mode 3 \
  --rendering-size <width> <height>

# With original init settings (match recording)
SIBR_gaussianViewer_app_d.exe \
  -m <path_to_3DGS_scene> \
  -in <output0.csv> \
  --rendering-mode 3 \
  --initial-position <X> <Y> <Z> \
  --initial-quaternion <QX> <QY> <QZ> <QW> \
  --initial-scale <S>
```

- Default size: 2064 × 2272 (per eye).

To replay a headset CSV in Desktop Mode (side-by-side), use `--rendering-mode 4 --rendering-size 2064 2272` (or your headset's openxr resolution)

---

## Example Scene Initialization Values

Pretrained 3DGS models can be downloaded from:

```textile
https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/pretrained/models.zip
```

Extract the file to your preferred path. To test with a Common initialization:

| Scene Name | Initial Position (X Y Z) | Quaternion (X Y Z W)           | Scale |
| ---------- | ------------------------ | ------------------------------ | ----- |
| truck      | `-2 1.8 -4`              | `-0.0872 0.0000 0.0000 0.9962` | 0.8   |

> These values apply to **Headset Mode only**. If replaying a trace recorded with specific settings, use identical `--initial-*` flags to avoid misaligned output.

For detailed initialization Parameters, please refer to Table 1 in the paper 👁️NavGS or the [`scene_setting.csv`](https://github.com/symmru/EyeNavGS_Rutgers_Dataset/blob/main/scene_setting.csv "scene_setting.csv") in the [EyeNavGS_Rutgers_Dataset](https://github.com/symmru/EyeNavGS_Rutgers_Dataset) .

- Example command for starting the SIBR viewer in desktop mode with the scene downloaded:
  
  ```bash
  SIBR_gaussianViewer_app_d.exe -m C:\User\SIBR\models\truck--rendering-mode 0
  ```

- Example command for starting the SIBR viewer in Headset mode with the scene downloaded and initialization:
  
  ```bash
  SIBR_gaussianViewer_app_d.exe -m D:\projects\sibr\scenes\models\truck --rendering-mode 2 --initial-position 0 2.1 -4 --initial-quaternion  -0.0896, 0.0000, 0.0000, 0.9960 --initial-scale 0.76
  ```

---

## EyeNavGS Utility Tools

Located in `utils/`, these utilities support post-processing and integration with other frameworks :

1. **`AddEyeGazeTracking/`**
   
   - Overlay eye-gaze positions onto per-eye VR recordings and then merge the left/right overlay outputs into one video.
     ![gaze-sample](./docs/img/bicycle-gaze-u101-p2.png)

2. **`JsonCsvTraceConvert/`**
   
   - Converts EyeNavGS CSV traces to JSON (4×4 homogeneous pose matrices) compatible with other viewers (e.g., NeRFstudio, SGSS).
   - Converts JSON traces (in external coordinate system) back into EyeNavGS CSV (undoing coordinate transforms).

3. **`WinSceneSelector/`**
   
   - For Windows, this script let user to easily launch various scenes in VR mode.

4. **`WorldCoordConvert/`**
   
   - Converts recorded “virtual world coordinates” back to 1:1 “physical stage coordinates” by applying the inverse of initialization transforms (tilt, scale, translation).

These tools ensure interoperability, reproducibility, and easy visualization of collected 👁️NavGS traces.

---

# SIBR Core

**SIBR** is a System for Image-Based Rendering.  
It is built around the *sibr-core* in this repo and several *Projects* implementing published research papers.  
For more complete documentation, see here: [SIBR Documentation](https://sibr.gitlabpages.inria.fr)
This **SIBR core** repository provides :

- a basic Image-Based Renderer

- a per-pixel implementation of Unstructured Lumigraph (ULR)

- several dataset tools & pipelines do process input images
  Details on how to run in the documentation and in the section below.  
  If you use this code in a publication, please cite the system as follows:
  
  ```
  @misc{sibr2020,
   author       = "Bonopera, Sebastien and Esnault, Jerome and Prakash, Siddhant and Rodriguez, Simon and Thonat, Theo and Benadel, Mehdi and Chaurasia, Gaurav and Philip, Julien and Drettakis, George",
   title        = "sibr: A System for Image Based Rendering",
   year         = "2020",
   url          = "https://gitlab.inria.fr/sibr/sibr_core"
  }
  ```
  
  ## OpenXR
  
  This branch supports headed-mounted displays through [OpenXR](#use-a-vr-headset).
  
  ## Setup
  
  **Note**: The current release is for *Windows 10* only. Please not that Visual Studio with c++20 standard is required to compile. We are planning a Linux release soon.
  
  #### Binary distribution
  
  The easiest way to use SIBR is to download the binary distribution. All steps described below, including all preprocessing for your datasets will work using this code.
  Download the distribution from the page: https://sibr.gitlabpages.inria.fr/download.html (Core, 57Mb); unzip the file and rename the directory "install".
  
  #### Install requirements

- [**Visual Studio 2019**](https://visualstudio.microsoft.com/fr/downloads/)

- [**Cmake 3.16+**](https://cmake.org/download)

- [**7zip**](https://www.7-zip.org)

- [**Python 3.8+**](https://www.python.org/downloads/) for shaders installation scripts and dataset preprocess scripts

- [**Doxygen 1.8.17+**](https://www.doxygen.nl/download.html#srcbin) for documentation

- [**CUDA 10.1+**](https://developer.nvidia.com/cuda-downloads) and [**CUDnn**](https://developer.nvidia.com/cudnn) if projects requires it
  Make sure Python, CUDA and Doxygen are in the PATH
  If you have Chocolatey, you can grab most of these with this command:
  
  ```sh
  choco install cmake 7zip python3 doxygen.install cuda
  ## Visual Studio is available on Chocolatey,
  ## though we do advise to set it from Visual Studio Installer and to choose your licensing accordingly
  choco install visualstudio2019community
  ```
  
  #### Generation of the solution

- Checkout this repository's master branch:
  
  ```sh
  ## through HTTPS
  git clone https://gitlab.inria.fr/sibr/sibr_core.git -b master
  ## through SSH
  git clone git@gitlab.inria.fr:sibr/sibr_core.git -b master
  ```

- Run Cmake-gui once, select the repo root as a source directory, `build/` as the build directory. Configure, select the Visual Studio C++ Win64 compiler

- Select the projects you want to generate among the BUILD elements in the list (you can group Cmake flags by categories to access those faster)

- Generate
  
  #### Compilation

- Open the generated Visual Studio solution (`build/sibr_projects.sln`)

- Build the `ALL_BUILD` target, and then the `INSTALL` target

- The compiled executables will be put in `install/bin`

- TODO: are the DLLs properly installed?
  
  #### Compilation of the documentation

- Open the generated Visual Studio solution (`build/sibr_projects.sln`)

- Build the `DOCUMENTATION` target

- Run `install/docs/index.html` in a browser
  
  ## Scripts
  
  Some scripts will require you to install `PIL`, and `convert` from `ImageMagick`.
  
  ```sh
  ## To install pillow
  python -m pip install pillow
  ## If you have Chocolatey, you can install imagemagick from this command
  choco install imagemagick
  ```
  
  ## Troubleshooting
  
  #### Bugs and Issues
  
  We will track bugs and issues through the Issues interface on gitlab. Inria gitlab does not allow creation of external accounts, so if you have an issue/bug please email `sibr@inria.fr` and we will either create a guest account or create the issue on our side.
  
  #### Cmake complaining about the version
  
  if you are the first to use a very recent Cmake version, you will have to update `CHECKED_VERSION` in the root `CmakeLists.txt`.
  
  #### Weird OpenCV error
  
  you probably selected the 32-bits compiler in Cmake-gui.
  
  #### `Cmd.exe failed with error 009` or similar
  
  make sure Python is installed and in the path.
  
  #### `BUILD_ALL` or `INSTALL` fail because of a project you don't really need
  
  build and install each project separately by selecting the proper targets.
  
  #### Error in CUDA headers under Visual Studio 2019
  
  make sure CUDA >= 10.1 (first version to support VS2019) is installed.
  
  ## To run an example
  
  For more details, please see the documentation: http://sibr.gitlabpages.inria.fr
  Download a dataset from: https://repo-sam.inria.fr/fungraph/sibr-datasets/
  e.g., the *sibr-museum-front* dataset in the *DATASETS_PATH* directory.
  
  ```
  wget https://repo-sam.inria.fr/fungraph/sibr-datasets/museum_front27_ulr.zip
  ```
  
  Once you have built the system or downloaded the binaries (see above), go to *install/bin* and you can run:
  
  ```
  sibr_ulrv2_app.exe --path DATASETS_PATH/sibr-museum-front
  ```
  
  You will have an interactive viewer and you can navigate freely in the captured scene. 
  Our default interactive viewer has a main view running the algorithm and a top view to visualize the position of the calibrated cameras. By default you are in WASD mode, and can toggle to trackball using the "y" key. Please see the page [Interface](https://sibr.gitlabpages.inria.fr/docs/nightly/howto_sibr_useful_objects.html) for more details on the interface.
  Please see the documentation on how to create a dataset from your own scene, and the various other IBR algorithms available.
  
  ### Support for VR headsets using OpenXR (provided by Orange)

- The new SIBR rendering mode `OpenXRRdrMode` supports Headed-Mounted dislay (HMD) OpenXR devices.

- The GaussianViewer can use this rendering mode with `--rendering-mode 2` option to render 3D Gaussian Splatting scene to two-view headset display through OpenXR stack.

- This mode works on Windows and Linux (through the SteamVR OpenXR runtime).
  Two VR experience modes are available:

- **Free world standing:** you can walk freely in a rectangular play space

- **Seated:** you can look around but only move within the space with controllers (the origin is world-locked)

**How to test:**

**Windows (Meta Quest 1/2/3/Pro):**

1. Install the Desktop PC Oculus Application: https://www.meta.com/en-gb/help/quest/articles/headsets-and-accessories/oculus-rift-s/install-app-for-link/

2. Setup Quest Link: https://www.meta.com/en-gb/help/quest/articles/headsets-and-accessories/oculus-link/set-up-link/

3. The headset should be in the Oculus AirLink Home screen (white background)

4. Run `gaussianViewer -m <dataset_path> --rendering-mode 2`

5. You can try `Free world standing` and `Seated` VR experiences

6. If you are experiencing lags in the headset, try to lower the rendering resolution by changing the `Down scale factor` slider value.

**Linux (through Steam):**

1. Install Steam and SteamVR
2. Make SteamVR the OpenXR default runtime (Settings > OpenXR > SET STEAMVR AS OPENXR RUNTIME)
3. Restart SteamVR
4. Run `gaussianViewer -m <dataset_path> --rendering-mode 2`

Tested with an HTC Vive Pro with `beta - SteamVR Beta Update` on Ubuntu distribution and Meta Quest 2 with `Oculus` on Windows 11.

---

## 👁️NavGS (EyeNavGS) BibTex

If you use this 👁️NavGS datase in your research, please cite:

```textile
@article{ding2025eyenavgs,
  title={EyeNavGS: A 6-DoF Navigation Dataset and Record-n-Replay Software for Real-World 3DGS Scenes in VR},
  author={Ding, Zihao and Lee, Cheng-Tse and Zhu, Mufeng and Guan, Tao and Sun, Yuan-Chun and Hsu, Cheng-Hsin and Liu, Yao},
  journal={arXiv preprint arXiv:2506.02380},
  year={2025}
}
```

---

## License

This repository is a **modified fork** of the [SIBR (System for Image-Based Rendering)](https://gitlab.inria.fr/sibr/sibr_core) software.  
It includes both:

- **Original code from SIBR**, licensed under its own terms (see the `LICENSE` file), and

- **New components and modifications** developed by the 👁️NavGS project, licensed under the **Apache License, Version 2.0**.  (see`LICENSE_EYENAVGS.md`file)

You may obtain a copy of the Apache License at:

> http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, the software distributed under this license is distributed on an **"AS IS" BASIS**,  
**WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND**, either express or implied.

Please refer to the `NOTICE` and `LICENSE_EYENAVGS.txt` files for details on which parts are covered under Apache 2.0.

---

Thank you for exploring 👁️NavGS software. We welcome contributions, issue reports, and feedback via the GitHub repository’s Issue Tracker.
