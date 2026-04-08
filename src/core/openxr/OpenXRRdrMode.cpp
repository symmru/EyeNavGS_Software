// Software Name : SIBR_core
// SPDX-FileCopyrightText: Copyright (c) 2023 Orange
// SPDX-License-Identifier: Apache 2.0
//
// This software is distributed under the Apache 2.0 License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

// SPDX-FileCopyrightText: 2025 Systems and Multimedia Lab @ Rutgers University
// SPDX-License-Identifier: Apache-2.0
//
// This file is part of the 👁️NavGS (EyeNavGS) project.
// It is a modified version of original code from the SIBR project.
// See LICENSE_EYENAVGS.md and NOTICE for details.

#include <core/graphics/GUI.hpp>
#include <core/assets/Resources.hpp>
#include <core/openxr/OpenXRRdrMode.hpp>
#include <core/openxr/SwapchainImageRenderTarget.hpp>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>


namespace sibr
{

#ifdef XR_USE_PLATFORM_XLIB
    XrGraphicsBindingOpenGLXlibKHR createXrGraphicsBindingOpenGLXlibKHR(Display *display, GLXDrawable drawable, GLXContext context)
    {
        return XrGraphicsBindingOpenGLXlibKHR{
            .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR,
            .xDisplay = display,
            .glxDrawable = drawable,
            .glxContext = context};
    }
#endif

#ifdef XR_USE_PLATFORM_WIN32
    XrGraphicsBindingOpenGLWin32KHR createXrGraphicsBindingOpenGLWin32KHR(HDC hdc, HGLRC hglrc)
    {
        // Windows C++ compiler does not support C99 designated initializers
        return XrGraphicsBindingOpenGLWin32KHR{
            XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR, // .type
            NULL,                                      // .next
            hdc,                                       // .hDC
            hglrc                                      // .hGLRC
        };
    }
#endif

    //OpenXRRdrMode::OpenXRRdrMode(sibr::Window &window, Eigen::Vector3f ipos, Eigen::Vector4f iq, float scale, const std::string& configFile)
    OpenXRRdrMode::OpenXRRdrMode(sibr::Window& window, Eigen::Vector3f ipos, Eigen::Vector4f iq, float scale, const std::string& configFile, const std::string& sceneName)
    {
        XRwindow = &window;
        m_quadShader.init("Texture",
                          sibr::loadFile(sibr::Resources::Instance()->getResourceFilePathName("texture.vp")),
                          sibr::loadFile(sibr::Resources::Instance()->getResourceFilePathName("texture.fp")));

        //output sceneName
        _sceneName = sceneName;
        if (_sceneName.empty()) {
            _sceneName = "unknownscene";
        }

        // Shader to render a red quad at world ground (xz plane)
        std::string vertexShader =
            SIBR_SHADER(420,
                uniform mat4 viewProj;
                uniform vec2 bounds;
                out gl_PerVertex {
                    vec4 gl_Position;
                };

                // Generate 1-unit quad position on xz plane
                void main() {
                    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2) - 1.0f;
	                gl_Position = viewProj * vec4(pos.x * bounds.x / 2.f, 0.f, pos.y * bounds.y / 2.f, 1.0);
                });
        std::string fragmentShader = SIBR_SHADER(420,
			out vec4 out_color;
		    void main(void) {
			    out_color = vec4(1.0, 0.0, 0.0, 0.8); // red semi-transparent quad
		    }
		);
		m_playSpaceShader.init("PlaySpace", vertexShader, fragmentShader);
		m_playSpaceParamVP.init(m_playSpaceShader,"viewProj");
		m_playSpaceBounds.init(m_playSpaceShader,"bounds");

        m_vrConfig = std::make_unique<VRConfiguration>(configFile);
        if (!m_vrConfig->load()) {
            SIBR_LOG << "No configuration file found for VR experience. Use default camera" << std::endl;
        }

        m_openxrHmd = std::make_unique<OpenXRHMD>("Gaussian splatting");
        
        m_openxrHmd->setInitialPose(ipos, iq, scale);
        if (!m_openxrHmd->init()) {
            SIBR_ERR << "Failed to connect to OpenXR" << std::endl;
        }
        bool sessionCreated = false;
#if defined(XR_USE_PLATFORM_XLIB)
        sessionCreated = m_openxrHmd->startSession(createXrGraphicsBindingOpenGLXlibKHR(glfwGetX11Display(), glXGetCurrentDrawable(), glfwGetGLXContext(window.GLFW())));
#elif defined(XR_USE_PLATFORM_WIN32)
        sessionCreated = m_openxrHmd->startSession(createXrGraphicsBindingOpenGLWin32KHR(wglGetCurrentDC(), wglGetCurrentContext()));
#endif
        if (!sessionCreated)
        {
            SIBR_ERR << "Failed to connect to OpenXR" << std::endl;
        }
        m_openxrHmd->startEyeTracker();
        SIBR_LOG << "Disable VSync: use headset synchronization." << std::endl;
        window.setVsynced(false);

        m_openxrHmd->setIdleAppCallback([this]()
                                        { m_appFocused = false; });
        m_openxrHmd->setVisibleAppCallback([this]()
                                           { m_appFocused = false; });
        m_openxrHmd->setFocusedAppCallback([this]()
                                           { m_appFocused = true; });

        if (m_openxrHmd->input()) {
            // Move camera with left stick
            m_openxrHmd->input()->setStickMoveCallback(OpenXRInput::Hand::LEFT, [this, iq](float x, float y) {
                /*
                float step = 0.05f;                
                
                Eigen::Vector3f forward =  currentQ * Eigen::Vector3f(0, 0, -1); //calculate forward based on current Quaternion
               
                if (abs(y) > 0.5f) {
                    Movement += (forward * y * step * m_controlSensitivity);
                }
            });
                */
                float step = 0.05f;
                Eigen::Vector3f up = currentQ * Eigen::Vector3f(0, 1, 0);
                if (abs(y) > 0.5f) {
                    Movement += (up * y * step * m_controlSensitivity);
                }
                });
            m_openxrHmd->input()->setStickMoveCallback(OpenXRInput::Hand::RIGHT, [this, iq](float x, float y) {
                /*
                float step = 0.05f;
                
                Eigen::Vector3f right = currentQ * Eigen::Vector3f(1, 0, 0);   //calculate right based on current Quaternion

                // Move camera with right horizontal stick
                if (abs(x) > 0.5f) {
                    Movement += (right * x * step * m_controlSensitivity);
                }
                // Elevate/lower camera with right vertical stick
                if (abs(y) > 0.5f) {
                    Eigen::Vector3f up = currentQ * Eigen::Vector3f(0, 1, 0); // calculate upward based on current Quaternion
                    Movement += (up * y * step * m_controlSensitivity);
                }
            });
            */
                float step = 0.05f;
                Eigen::Vector3f right = currentQ * Eigen::Vector3f(1, 0, 0);
                Eigen::Vector3f forward = currentQ * Eigen::Vector3f(0, 0, -1);
                if (abs(x) > 0.5f) {
                    Movement += (right * x * step * m_controlSensitivity);
                }
                if (abs(y) > 0.5f) {
                    Movement += (forward * y * step * m_controlSensitivity);
                }
                });

            // Move scene with left hand drag (position + trigger)
           /* m_openxrHmd->input()->setTriggerCallback(OpenXRInput::Hand::LEFT, [this](float val) {
                const Vector3f& handPose = vrToWorld(m_openxrHmd->input()->getHandPosePosition(OpenXRInput::Hand::LEFT));
                m_leftTriggerPressed = val > 0.5f;
                if (m_leftTriggerPressed)
                {
                    Vector3f t =  (handPose - m_prevLeftHandPosition);
                    m_vrConfig->sceneTransform().translate(t);
                }
                m_prevLeftHandPosition = handPose;

            });*/
            // Rotate scene with right hand drag (orientation + trigger)
          /*  m_openxrHmd->input()->setTriggerCallback(OpenXRInput::Hand::RIGHT, [this](float val) {
                const Quaternionf& handRotation = vrToWorld(m_openxrHmd->input()->getHandPoseOrientation(OpenXRInput::Hand::RIGHT));
                m_rightTriggerPressed = val > 0.5f;
                if (m_rightTriggerPressed)
                {
                    Quaternionf diff = m_prevRightHandOrientation.slerp(0.1f, handRotation) * m_prevRightHandOrientation.inverse();
                    m_vrConfig->sceneTransform().rotate(diff.inverse());
                }
                m_prevRightHandOrientation = handRotation;
            });*/
        }
    }

    OpenXRRdrMode::~OpenXRRdrMode()
    {/**/
        m_RTPool.clear();
        m_openxrHmd->closeSession();
        m_openxrHmd->terminate();
    }

     //Load Trace from CSV file  
    void OpenXRRdrMode::loadViewData(ViewData& viewData)
    { 
        
        std::string line;
        //get the next line every frame
        if (std::getline(inFile, line)) {
            //using a stream to store the line
            std::istringstream lineStream(line);
            std::string token;

            //miss the ViewIndex
            std::getline(lineStream, token, ',');
            

            // Read FOV
            for (int i = 0; i < 4; ++i) {
                std::getline(lineStream, token, ',');
                viewData.fov(i) = std::stof(token);
            }

            // Read Position
            for (int i = 0; i < 3; ++i) {
                std::getline(lineStream, token, ',');
                viewData.position(i) = std::stof(token);
            }

            // Read Quaternion
            for (int i = 0; i < 4; ++i) {
                std::getline(lineStream, token, ',');
                viewData.quaternion.coeffs()(i) = std::stof(token);
            }
        }
        else {

            if (leftEyeVideoWriter.isOpened()) {
                leftEyeVideoWriter.release();
            }
            if (rightEyeVideoWriter.isOpened()) {
                rightEyeVideoWriter.release();
            }
            // Handle the end of file or read error
            PlayMode = 1;
            inFile.close();
            SIBR_LOG << "Replay Finished!" << std::endl;
            exit(0);
        }
    }


    void OpenXRRdrMode::render(ViewBase &view, const sibr::Camera &camera, const sibr::Viewport &viewport, IRenderTarget *optDest)
    {
        // Render the UI with OpenXR infos
        onGui();

        if (!m_openxrHmd->isSessionRunning())
        {
            return;
        }

        // Get next pose prediction for rendering
        m_openxrHmd->pollEvents();
        if (!m_openxrHmd->waitNextFrame())
        {
            return;
        }

        // Headset pose has changed, let's update our VR head camera
        updateHeadCamera(camera);

        const int w = m_openxrHmd->getResolution().x();
        const int h = m_openxrHmd->getResolution().y();

        // Prepare the view to render at a specific resolution
        view.setResolution(sibr::Vector2i(w / m_downscaleResolution, h / m_downscaleResolution));

        // The callback is called for each single view (left view then right view) with the texture to render to
        m_openxrHmd->submitFrame([this, w, h, &view, optDest](int viewIndex, uint32_t texture)
                                 {
                                     OpenXRHMD::Eye eye = viewIndex == 0 ? OpenXRHMD::Eye::LEFT : OpenXRHMD::Eye::RIGHT;
                                    /*
                                     auto fov = this->m_openxrHmd->getFieldOfView(eye);
                                     auto q = this->m_openxrHmd->getPoseQuaternion(eye);
                                     auto pos = this->m_openxrHmd->getPosePosition(eye);
                                    */
                                     //Call loadViewData to replay
                                     
                                     ViewData viewData;
                                     if (PlayMode == 2) {
                                         loadViewData(viewData);
                                         
                                     }
                                     //playMode == 2, Replay; playMode == 0 or 1, HMD controls;
                                     auto fov = PlayMode == 2 ? viewData.fov : this->m_openxrHmd->getFieldOfView(eye);
                                     auto q = PlayMode == 2 ? viewData.quaternion : this->m_openxrHmd->getPoseQuaternion(eye);
                                     auto pos = PlayMode == 2 ? viewData.position : this->m_openxrHmd->getPosePosition(eye);

                                     if (!m_openxrHmd->eyeGazes.gaze[viewIndex].isValid && isEyeTracking) {
                                         SIBR_LOG<< "Eye Gaze is invalid!\n";
                                         isEyeTracking = false;
                                      }
                                     XrQuaternionf unitQ = { 0,0,0,1 };
                                     XrVector3f unitP = { 0,0,0 };
                                     auto gaze_q = (isEyeTracking) ? m_openxrHmd->eyeGazes.gaze[viewIndex].gazePose.orientation : unitQ;
                                     auto gaze_pos = (isEyeTracking) ? m_openxrHmd->eyeGazes.gaze[viewIndex].gazePose.position : unitP;
                                     currentQ = q;
                                     pos += Movement;
                                     gaze_pos.x += Movement.x();
                                     gaze_pos.y += Movement.y();
                                     gaze_pos.z += Movement.z();

                                     // Get timestamp in microseconds
                                     using namespace std::chrono;
                                                                          
                                     auto now = high_resolution_clock::now();
                                     auto now_us = time_point_cast<microseconds>(now);
                                     auto epoch = now_us.time_since_epoch().count();
                                     double epoch_ms = static_cast<double>(epoch) / 1000.0;


                                     // Get realtime with millisecond precision
                                     auto now_sys = system_clock::now();
                                     auto duration = now_sys.time_since_epoch();
                                     auto millis = duration_cast<milliseconds>(duration).count();
                                     auto seconds_part = duration_cast<std::chrono::seconds>(duration);
                                     auto ms_part = millis % 1000;

                                     std::time_t time_now = system_clock::to_time_t(now_sys);
                                     std::tm tm_now;
                                     localtime_s(&tm_now, &time_now);

                                     std::ostringstream realtimeStream;
                                     realtimeStream << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms_part;

                                     //Compute elapsed_ms
                                     auto now_steady = std::chrono::steady_clock::now();
                                     //auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now_steady - recordingStartTime).count();
                                     if (recordingStartTime.time_since_epoch().count() == 0) {
                                         recordingStartTime = now_steady;
                                     }

                                     auto elapsed = std::chrono::duration<double, std::milli>(now_steady - recordingStartTime).count();
                                     
                                     //Get ISO 8601 timestamp with local timezone 
                                     /*
                                     auto now_sys = std::chrono::system_clock::now();
                                     auto duration = now_sys.time_since_epoch();
                                     auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                                     auto ms_part = millis % 1000;

                                     std::time_t now_tt = std::chrono::system_clock::to_time_t(now_sys);
                                     std::tm local_tm;
                                     localtime_s(&local_tm, &now_tt);  // Windows-safe
                                     */

                                     std::ostringstream isoStream;
                                     isoStream << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%S")
                                         << '.' << std::setw(3) << std::setfill('0') << ms_part;

                                     // Timezone offset
                                     //long timezone_offset_sec = _timezone;
                                     long timezone_offset_sec = 0;
                                     _get_timezone(&timezone_offset_sec);

                                     int offset_hours = -timezone_offset_sec / 3600;
                                     int offset_minutes = (abs(timezone_offset_sec) / 60) % 60;

                                     isoStream << (offset_hours >= 0 ? '+' : '-')
                                         << std::setw(2) << std::setfill('0') << abs(offset_hours)
                                         << ":" << std::setw(2) << std::setfill('0') << offset_minutes;

                                     
                                     
                                     //-----save position and quaternion to file

                                     /*
                                     
                                     if (PlayMode == 0||Rec_Sav) {
                                         outFile << viewIndex << "," // View index
                                             << fov.x() << "," << fov.y() << "," << fov.z() << "," << fov.w() << ","
                                             << pos.x() << "," << pos.y() << "," << pos.z() << ","
                                             << q.x() << "," << q.y() << "," << q.z() << "," << q.w() << ","
                                             << gaze_q.x << "," << gaze_q.y << "," << gaze_q.z << "," << gaze_q.w << ","
                                             << gaze_pos.x << "," << gaze_pos.y << "," << gaze_pos.z
                                             << "\n";
                                     }
                                     */

                                     outFile << viewIndex << ","
                                         << fov.x() << "," << fov.y() << "," << fov.z() << "," << fov.w() << ","
                                         << pos.x() << "," << pos.y() << "," << pos.z() << ","
                                         << q.x() << "," << q.y() << "," << q.z() << "," << q.w() << ","
                                         << gaze_q.x << "," << gaze_q.y << "," << gaze_q.z << "," << gaze_q.w << ","
                                         << gaze_pos.x << "," << gaze_pos.y << "," << gaze_pos.z << ","
                                         << epoch << "," << realtimeStream.str() << ","
                                         //<< elapsed << "," << isoStream.str()
                                         << std::fixed << std::setprecision(3) << elapsed << "," << isoStream.str() << "," 
                                         << std::fixed << std::setprecision(3) << epoch_ms
                                         << "\n";



                                     // OpenXR eye position is in world coordinates system (+x: right, +y: up; +z: backward)
                                     // 3DGS reference scenes have the following coordinate system : +x: right, +y: down, +z: forward
                                     // Let's rotate the camera to have the right-side up scene
                                     if (m_flipY)
                                     {
                                         Eigen::Matrix3f mat;
                                         mat << 1.0f, 0.0f, 0.0f,
                                             0.0f, -1.0f, 0.0f,
                                             0.0f, 0.0f, -1.0f;
                                         Eigen::Matrix3f rot = mat * q.matrix();
                                         q = Eigen::Quaternionf(rot);
                                         pos = mat * pos;
                                     }



                                     // Define camera from OpenXR eye view position/orientation/fov
                                     Camera cam;
                                     
                                     cam.rotate(q);
                                     cam.position(pos);
                                     cam.zfar(0.2f);
                                     cam.znear(250.f);
                                     cam.fovy(fov.w() - fov.z());
                                     cam.aspect((fov.y() - fov.x()) / (fov.w() - fov.z()));

                                     if (m_vrExperience == 1) { // 1: seated experience - used sibr_viewer current camera's position as default position
                                         cam.translate(cam.position());
                                     }

                                     // Note: setStereoCam() used in SteroAnaglyph canno be reused here,
                                     // because headset eye views have asymetric fov
                                     // We therefore use the perspective() method with principal point positioning instead
                                     // 
                                     cam.principalPoint(Eigen::Vector2f(1.f, 1.f) - this->m_openxrHmd->getScreenCenter(eye));
                                     cam.perspective(cam.fovy(), (float)w / (float)h, cam.znear(), cam.zfar());

                                     // Get the render target holding the swapchain image's texture from the pool
                                     auto rt = getRenderTarget(texture, w, h);
                                     if (!rt)
                                     {
                                         return;
                                     }
                                     rt->clear();
                                     rt->bind();
                                     glViewport(0, 0, w, h);
                                     view.onRenderIBR(*rt.get(), cam);
                                     rt->unbind();
                                     

                                     // Draw the left and right textures into the UI window
                                     if (optDest)
                                     {
                                         glViewport(eye == OpenXRHMD::Eye::LEFT ? 0 : optDest->w() / 2, 0, optDest->w() / 2, optDest->h());
                                         glScissor(eye == OpenXRHMD::Eye::LEFT ? 0 : optDest->w() / 2, 0, optDest->w() / 2, optDest->h());
                                         optDest->bind();
                                     }
                                     else
                                     {
                                         glViewport(eye == OpenXRHMD::Eye::LEFT ? 0 : w / 2, 0, w / 2, h);
                                         glScissor(eye == OpenXRHMD::Eye::LEFT ? 0 : w / 2, 0, w / 2, h);
                                     }
                                     glEnable(GL_SCISSOR_TEST);
                                     glDisable(GL_BLEND);
                                     glDisable(GL_DEPTH_TEST);
                                     glClearColor(0.f, 0.f, 0.f, 1.f);
                                     glClear(GL_COLOR_BUFFER_BIT);
                                     m_quadShader.begin();
                                     glActiveTexture(GL_TEXTURE0);
                                     glBindTexture(GL_TEXTURE_2D, texture);
                                     RenderUtility::renderScreenQuad();
                                     glBindTexture(GL_TEXTURE_2D, 0);
                                     m_quadShader.end();
                                     glDisable(GL_SCISSOR_TEST);
                                     if (optDest)
                                     {
                                         optDest->unbind();
                                     }

                                     if (PlayMode==2||Rec_Sav) {

                                         glBindTexture(GL_TEXTURE_2D, texture);

                                         // create the buffer to store the data
                                         std::vector<GLubyte> pixels(w * h * 3);

                                         // load pixel data from GPU to buffer
                                         glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

                                         // convert pixel data to OpenCV Mat Object
                                         cv::Mat frame(h, w, CV_8UC3, pixels.data());

                                         // OpenCV default using BGR format，thus convert it to RGB
                                         cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
                                          cv::flip(frame, frame, 0); 
                                         // write frame to different video according to the viewIndex(0 is left, 1 is right)
                                         if (viewIndex == 0 && leftEyeVideoWriter.isOpened()) {
                                             leftEyeVideoWriter.write(frame); // left eye
                                         }
                                         else if (viewIndex == 1 && rightEyeVideoWriter.isOpened()) {
                                             rightEyeVideoWriter.write(frame); // right eye
                                         }

                                         glBindTexture(GL_TEXTURE_2D, 0);
                                     }

                                 });
    }

    void OpenXRRdrMode::StartRecord(const std::string& saveFilePath) {
         
            const int w = m_openxrHmd->getResolution().x();
            const int h = m_openxrHmd->getResolution().y();
            int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4'); // using MJPG format
            int fps = 30; // 30 frame per second
            cv::Size frameSize(w, h); // video size
            size_t pos = saveFilePath.find_last_of(".");

            const std::string File= saveFilePath.substr(0, pos);
            // video file name is File+left/right
            //leftEyeVideoWriter.open(File+"left.mp4", fourcc, fps, frameSize);
            //rightEyeVideoWriter.open(File+"right.mp4", fourcc, fps, frameSize);

            leftEyeVideoWriter.open(_sceneName + "_left.mp4", fourcc, fps, frameSize);
            rightEyeVideoWriter.open(_sceneName + "_right.mp4", fourcc, fps, frameSize);


            if (!Rec_Sav) {
                if (!inFile.is_open()) {
                    inFile.open(saveFilePath);
                    // Skip the header line
                    std::string header;
                    std::getline(inFile, header);
                }

                if (inFile.is_open()) {
                    PlayMode = 2;
                    SIBR_LOG << "Start Replay---by using the trace!" << std::endl;
                }
                else {
                    SIBR_LOG << "Do not find path file to replay!" << std::endl;
                }
            }
    }
    void OpenXRRdrMode::onGui()
    {
        const std::string guiName = "OpenXR";
        ImGui::Begin(guiName.c_str());
        std::string status = "KO";
        if (m_openxrHmd->isSessionRunning())
        {
            status = m_appFocused ? "FOCUSED" : "IDLE";
        }
        ImGui::Text("Session status: %s", status.c_str());
        ImGui::Text("Runtime: %s (%s)", m_openxrHmd->getRuntimeName().c_str(), m_openxrHmd->getRuntimeVersion().c_str());
        ImGui::Text("Reference space type: %s", m_openxrHmd->getReferenceSpaceType());
        ImGui::RadioButton("Free world standing", &m_vrExperience, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Seated", &m_vrExperience, 1);
        ImGui::Checkbox("Y-Invert scene", &m_flipY);

        // Add Save Tracks button
        if (ImGui::Button("Start Trace Recording")) {
            PlayMode = 0;
            recordingStartTime = std::chrono::steady_clock::now();

            
            if (!outFile.is_open()) {
                //const std::string& out = "Output" + std::to_string(FrameIndex++)+".csv";
                const std::string out = _sceneName + "_trace_" + std::to_string(FrameIndex++) + ".csv";
                outFile.open(out, std::ios::app);
                // Write the CSV header if the file is being created
                /*
                if (outFile.tellp() == 0) {
                    outFile << "ViewIndex,FOV1,FOV2,FOV3,FOV4,PositionX,PositionY,PositionZ,QuaternionX,QuaternionY,QuaternionZ,QuaternionW,GazeQX,GazeQY,GazeQZ,GazeQW,GazePosX,GazePosY,GazePosZ\n";
                }
                */
                if (outFile.tellp() == 0) {
                    outFile << "ViewIndex,FOV1,FOV2,FOV3,FOV4,PositionX,PositionY,PositionZ,QuaternionX,QuaternionY,QuaternionZ,QuaternionW,GazeQX,GazeQY,GazeQZ,GazeQW,GazePosX,GazePosY,GazePosZ,timestamp,realtime,elapsed_ms,iso_time,epoch_ms\n";
                }

            }
            
            SIBR_LOG << "Start saving HMD traces to output file" << std::endl;
        }

        // Add End Trace Recording button
        ImGui::SameLine();  // Display on the same line
        if (ImGui::Button("End Trace Recording")) {
            if (outFile.is_open()) {
                outFile.close();
            }

            if (leftEyeVideoWriter.isOpened()) {
                leftEyeVideoWriter.release();
            }
            if (rightEyeVideoWriter.isOpened()) {
                rightEyeVideoWriter.release();
            }
            Rec_Sav = false;
            PlayMode = 1;
            SIBR_LOG << "Saving Finished" << std::endl;
            
        }
        //Add Recording and Saving button
        if (ImGui::Button("Recording and Saving") ){
            PlayMode = 0;
            Rec_Sav = true;
            recordingStartTime = std::chrono::steady_clock::now();

            StartRecord("output_Rec_");
            if (!outFile.is_open()) {
                //const std::string& out = "Output" + std::to_string(FrameIndex++) + ".csv";
                const std::string out = _sceneName + "_output_" + std::to_string(FrameIndex++) + ".csv";
                outFile.open(out, std::ios::app);
                // Write the CSV header if the file is being created
                if (outFile.tellp() == 0) {
                    outFile << "ViewIndex,FOV1,FOV2,FOV3,FOV4,PositionX,PositionY,PositionZ,QuaternionX,QuaternionY,QuaternionZ,QuaternionW,GazeQX,GazeQY,GazeQZ,GazeQW,GazePosX,GazePosY,GazePosZ\n";
                }
            }

            SIBR_LOG << "Start saving HMD traces to output file" << std::endl;
        }

        if (m_openxrHmd->isSessionRunning())
        {
            const auto report = m_openxrHmd->getRefreshReport();
            ImGui::Text("Framerate: %.2f FPS (expected: %.2f FPS)", report.measuredFramerate, report.expectedFramerate);
            const auto w = m_openxrHmd->getResolution().x();
            const auto h = m_openxrHmd->getResolution().y();
            ImGui::Text("Headset resolution (per eye): %ix%i", w, h);
            ImGui::Text("Rendering resolution (per eye): %ix%i", w / m_downscaleResolution, h / m_downscaleResolution);
			ImGui::SliderInt("Down scale factor", &m_downscaleResolution, 1, 8);
            ImGui::Text("IPD: %.1fcm", m_openxrHmd->getInterPupillaryDistance() * 100.f);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped))
            {
                ImGui::SetTooltip("Inter-pupillary distance");
            }
            ImGui::Checkbox("Show VR play space", &m_forceRenderVRPlaySpace);
            if (ImGui::Button("Save VR configuration"))
            {
                if (m_vrConfig->save())
                {
                    SIBR_LOG << "VR configuration saved to '" << m_vrConfig->filePath() << "'" << std::endl;
                }
            }
            if (ImGui::CollapsingHeader("Controls"))
            {
			    ImGui::SliderFloat("Sensitivity", &m_controlSensitivity, 0.1f, 1.f);
                ImGui::Text("Move camera: Left stick or");
                ImGui::SameLine();
                if (ImGui::Button("Left"))
                {
                    m_vrConfig->camera().translate((m_vrConfig->camera().rotation() * m_headCameraInVrWorld.right() * 1.0f) * m_controlSensitivity);
                }
                ImGui::SameLine();
                if (ImGui::Button("Right"))
                {
                    m_vrConfig->camera().translate((m_vrConfig->camera().rotation() * m_headCameraInVrWorld.right()) * m_controlSensitivity);
                }
                ImGui::SameLine();
                if (ImGui::Button("Forward"))
                {
                    m_vrConfig->camera().translate((m_vrConfig->camera().rotation() * m_headCameraInVrWorld.dir()) * m_controlSensitivity);
                }
                ImGui::SameLine();
                if (ImGui::Button("Backward"))
                {
                    m_vrConfig->camera().translate((m_vrConfig->camera().rotation() * m_headCameraInVrWorld.dir() * 1.0f) * m_controlSensitivity);
                }
                ImGui::Text("Elevate/lower camera: Right vertical stick or");
                ImGui::SameLine();
                if (ImGui::Button("Up"))
                {
                    m_vrConfig->camera().translate(m_vrConfig->camera().up() * m_controlSensitivity);
                }
                ImGui::SameLine();
                if (ImGui::Button("Down"))
                {
                    m_vrConfig->camera().translate(-m_vrConfig->camera().up() * m_controlSensitivity);
                }
                ImGui::Text("Rotate camera: Right horizontal stick or");
                ImGui::SameLine();
                if (ImGui::Button("Rotate CW"))
                {
                    m_vrConfig->camera().rotate(Quaternionf(Eigen::AngleAxisf(-m_controlSensitivity, m_vrConfig->camera().up())));
                }
                ImGui::SameLine();
                if (ImGui::Button("Rotate ACW"))
                {
                    m_vrConfig->camera().rotate(Quaternionf(Eigen::AngleAxisf(m_controlSensitivity, m_vrConfig->camera().up())));
                }
                ImGui::Text("Move scene: drag with left controller");
                ImGui::Text("Rotate scene: drag with right controller");
            }
            if (ImGui::CollapsingHeader("Debug"))
            {
                ImGui::Text("Left eye:");
                const auto leftPos = this->m_openxrHmd->getPosePosition(OpenXRHMD::Eye::LEFT);
                const auto leftRot = this->m_openxrHmd->getPoseOrientation(OpenXRHMD::Eye::LEFT, OpenXRHMD::AngleUnit::DEGREE);
                const auto rightPos = this->m_openxrHmd->getPosePosition(OpenXRHMD::Eye::RIGHT);
                const auto rightRot = this->m_openxrHmd->getPoseOrientation(OpenXRHMD::Eye::RIGHT, OpenXRHMD::AngleUnit::DEGREE);
                auto fov = m_openxrHmd->getFieldOfView(OpenXRHMD::Eye::LEFT, OpenXRHMD::AngleUnit::DEGREE);
                ImGui::Text("\tFOV: %.2f°, %.2f°, %.2f°, %.2f°", fov.x(), fov.y(), fov.z(), fov.w());
                ImGui::Text("\tPosition : %.2f, %.2f, %.2f", leftPos.x(), leftPos.y(), leftPos.z());
                ImGui::Text("\tOrientation : %.2f, %.2f, %.2f", leftRot.x(), leftRot.y(), leftRot.z());
                ImGui::Text("Right eye:");
                fov = m_openxrHmd->getFieldOfView(OpenXRHMD::Eye::RIGHT, OpenXRHMD::AngleUnit::DEGREE);
                ImGui::Text("\tFOV: %.2f°, %.2f°, %.2f°, %.2f°", fov.x(), fov.y(), fov.z(), fov.w());
                ImGui::Text("\tPosition : %.2f, %.2f, %.2f", rightPos.x(), rightPos.y(), rightPos.z());
                ImGui::Text("\tOrientation : %.2f, %.2f, %.2f", rightRot.x(), rightRot.y(), rightRot.z());
            }
        }
        ImGui::End();
    }

    SwapchainImageRenderTarget::Ptr OpenXRRdrMode::getRenderTarget(uint32_t texture, uint w, uint h)
    {
        auto i = m_RTPool.find(texture);
        if (i != m_RTPool.end())
        {
            return i->second;
        }
        else
        {
            SwapchainImageRenderTarget::Ptr newRt = std::make_shared<SwapchainImageRenderTarget>(texture, w, h);
            auto pair = m_RTPool.insert(std::make_pair<int, SwapchainImageRenderTarget::Ptr>(texture, std::move(newRt)));
            if (pair.second)
            {
                return (*pair.first).second;
            }
        }
        return SwapchainImageRenderTarget::Ptr();
    }

    Camera createCamera(float znear, float zfar, const Vector3f& position, const Quaternionf& rotation) {
        Camera cam;
        cam.znear(znear);
        cam.zfar(zfar);
        cam.position(position);
        cam.rotation(rotation);
        return cam;
    }

    Vector3f OpenXRRdrMode::vrToWorld(const Vector3f& pos) const {
        return m_vrConfig->camera().rotation() * pos + m_vrConfig->camera().position();
    }

    Quaternionf OpenXRRdrMode::vrToWorld(const Quaternionf& quat) const {
        return m_vrConfig->camera().rotation() * quat;
    }
    
    void OpenXRRdrMode::updateHeadCamera(const Camera& camera)
    {
        // If not set, initialize the VR configuration from the current camera
        if (!m_vrConfig->isSet())
        {
            Camera origin = camera;
             // ignore current camera orientation but flip Y and Z to respect SIBR camera coordinate convention (180° rotation about x)
            origin.rotation(Quaternionf { 0.f, 1.0f, 0.f, 0.f});
            m_vrConfig->setCamera(origin);
            m_vrConfig->sceneTransform().set(Vector3f(), Quaternionf::Identity());
        }

        auto znear = m_vrConfig->camera().znear();
        auto zfar = m_vrConfig->camera().zfar();

        // Cam (in VR world coordinates) used for VR ground rendering
        m_headCameraInVrWorld = createCamera(znear, zfar, m_openxrHmd->getHeadPosePosition(), m_openxrHmd->getHeadPoseQuaternion());

        // Transform relative VR pose to world coordinates in respect to VR config inital camera
        Vector3f headPosition;
        if (m_vrExperience == 0) { // free world => full VR experience
            headPosition = vrToWorld(m_headCameraInVrWorld.position());
            // Make head y position only depends on headpose height to not cumulate with the vr config camera's height
            headPosition.y() -= m_vrConfig->camera().position().y();
        } else { // seated => ignore headset position (video 360 experience)
            headPosition = m_vrConfig->camera().position();
        }
        Quaternionf headRotation = m_vrConfig->camera().rotation() * m_headCameraInVrWorld.rotation();

        // Apply scene transform
        Quaternionf q = m_vrConfig->sceneTransform().rotation().inverse();
        Vector3f t = -1.0f * (q * m_vrConfig->sceneTransform().position());
        // Cam with scene transform in world coordinates (used for rendering)
        m_headCamera = createCamera(znear, zfar, q * headPosition + t, q * headRotation);
    }


    Camera OpenXRRdrMode::computeEyeCam(const Camera& cam, OpenXRHMD::Eye eye) const {
        Camera eyeCam = cam;
        Vector3f parallaxShift = m_openxrHmd->getInterPupillaryDistance() / 2.f * eyeCam.right();
        if (eye == OpenXRHMD::Eye::LEFT)
        {
            parallaxShift *= -1.f;
        }
        eyeCam.translate(parallaxShift);
        const auto& fov = m_openxrHmd->getFieldOfView(eye);
        eyeCam.fovy(fov.w() - fov.z());
        eyeCam.aspect((fov.y() - fov.x()) / (fov.w() - fov.z()));
        // Note: setStereoCam() used in SteroAnaglyph cannot be reused here,
        // because headset eye views have asymetric fov
        // We therefore use the perspective() method with principal point positioning instead
        eyeCam.principalPoint(Eigen::Vector2f(1.f, 1.f) - m_openxrHmd->getScreenCenter(eye));
        eyeCam.perspective(eyeCam.fovy(), eyeCam.aspect(), eyeCam.znear(), eyeCam.zfar());
        return eyeCam;
    }

    void OpenXRRdrMode::renderVRPlaySpace(OpenXRHMD::Eye eye) {
        Camera vrCam = computeEyeCam(m_headCameraInVrWorld, eye);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_playSpaceShader.begin();
        m_playSpaceParamVP.set(vrCam.viewproj());
        m_playSpaceBounds.set(Vector2f {m_openxrHmd->getPlaySpaceBounds().x(), m_openxrHmd->getPlaySpaceBounds().y()});
        RenderUtility::useDefaultVAO();
        const unsigned char indices[] = {0, 1, 2, 3};
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);
        CHECK_GL_ERROR;
        m_playSpaceShader.end();
        glDisable(GL_BLEND);
    }

} /*namespace sibr*/
