#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "tiny_gltf.h"

#include <tools/logging.h>

#include "av_cef_app.h"
#include "avserver.h"

#include <chrono>
#include <thread>
#include <tools/systools.h>
#include <tools/pathtools.h>
#include <tools/stringtools.h>

// XXX

#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <filesystem>
#include <functional>

// #define CINTERFACE
// #define D3D11_NO_HELPERS
#include <windows.h>
// #include <D3D11_4.h>
// #include <DXGI1_4.h>
// #include <wrl.h>

// #include "device/vr/detours/detours.h"
#include "json.hpp"

// #include "device/vr/openvr/test/out.h"
// #include "third_party/openvr/src/src/vrcommon/sharedlibtools_public.h"
// #include "device/vr/openvr/test/fake_openvr_impl_api.h"
#include "base64.h"

#include "javascript_renderer.h"

#include "steam_api.h"
// #include "isteamhtmlsurface.h"

#include "out.h"
#include "io.h"

using json = nlohmann::json;
using Base64 = macaron::Base64;

std::string logSuffix = "_native";
// HWND g_hWnd = NULL;
// CHAR s_szDllPath[MAX_PATH] = "vrclient_x64.dll";
std::string dllDir;

inline uint32_t divCeil(uint32_t x, uint32_t y) {
  return (x + y - 1) / y;
}
constexpr uint32_t chunkSize = 1000*1000;
void respond(const json &j) {
  std::string outString = j.dump();
  uint32_t outSize = (uint32_t)outString.size();
  if (outSize < chunkSize) {
    std::cout.write((char *)&outSize, sizeof(outSize));
    std::cout.write(outString.data(), outString.size());
  } else {
    uint32_t numChunks = divCeil(outSize, chunkSize);
    // std::cout << "write chunks " << outSize << " " << chunkSize << " " << numChunks << std::endl;
    for (uint32_t i = 0; i < numChunks; i++) {
      // std::cout << "sending " << i << " " << numChunks << " " << outString.substr(i*chunkSize, chunkSize).size() << std::endl;
      json j2 = {
        {"index", i},
        {"total", numChunks},
        {"continuation", outString.substr(i*chunkSize, chunkSize)},
      };
      std::string outString2 = j2.dump();
      uint32_t outSize2 = (uint32_t)outString2.size();
      std::cout.write((char *)&outSize2, sizeof(outSize2));
      std::cout.write(outString2.data(), outString2.size());
    }
    // std::cout << "done sending" << std::endl;
  }
}

class CHTMLSurface {
public:
  CHTMLSurface(std::function<void(uint32_t width, uint32_t height, const char *data)> onPaint);

  // STEAM_CALLBACK( CHTMLSurface, OnBrowserReady, HTML_BrowserReady_t );
  STEAM_CALLBACK( CHTMLSurface, OnStartRequest, HTML_StartRequest_t ); // REQUIRED
  STEAM_CALLBACK( CHTMLSurface, OnJSAlert, HTML_JSAlert_t ); // REQUIRED
  STEAM_CALLBACK( CHTMLSurface, OnJSConfirm, HTML_JSConfirm_t ); // REQUIRED
  STEAM_CALLBACK( CHTMLSurface, OnUploadLocalFile, HTML_FileOpenDialog_t ); // REQUIRED
  STEAM_CALLBACK( CHTMLSurface, OnNeedsPaint, HTML_NeedsPaint_t ); // REQUIRED
  STEAM_CALLBACK( CHTMLSurface, OnNewWindow, HTML_NewWindow_t ); // REQUIRED
  STEAM_CALLBACK( CHTMLSurface, OnURLChanged, HTML_URLChanged_t ); // REQUIRED
  STEAM_CALLBACK( CHTMLSurface, OnBrowserRestarted, HTML_BrowserRestarted_t ); // REQUIRED

  void OnBrowserReady( HTML_BrowserReady_t *pBrowserReady, bool bIOFailure );
  CCallResult< CHTMLSurface, HTML_BrowserReady_t > m_SteamCallResultBrowserReady;
  std::function<void(uint32_t width, uint32_t height, std::vector<unsigned char> &&data)> onPaint;
  SteamAPICall_t browser;
  // vr::VROverlayHandle_t overlayHandle;
};
CHTMLSurface::CHTMLSurface(std::function<void(uint32_t width, uint32_t height, const char *data)> onPaint) :
  onPaint(onPaint)
{
  auto html = SteamHTMLSurface();
  getOut() << "got html " << (void *)html << std::endl;
  html->Init();
  browser = html->CreateBrowser(nullptr, nullptr);
  m_SteamCallResultBrowserReady.Set(browser, this, &CHTMLSurface::OnBrowserReady);
  getOut() << "got browser " << browser << std::endl;

  /* vr::VROverlayError error = vr::VROverlay()->CreateOverlay("browser", "browser", &overlayHandle);
  getOut() << "overlay create result " << error << std::endl;
  error = vr::VROverlay()->ShowOverlay(overlayHandle); */
}
void CHTMLSurface::OnBrowserReady(HTML_BrowserReady_t *pBrowserReady, bool bIOFailure) {
  getOut() << "browser ready 1 " << bIOFailure << std::endl;
  
  browser = pBrowserReady->unBrowserHandle;

  auto html = SteamHTMLSurface();
  html->SetSize(browser, 1280, 1280);
  // html->SetDPIScalingFactor(browser, 1.0f);
  html->LoadURL(browser, "https://google.com/", nullptr);
  
  getOut() << "browser ready 2 " << bIOFailure << std::endl;
}
void CHTMLSurface::OnStartRequest(HTML_StartRequest_t *pParam) {
  getOut() << "start request" << std::endl;
  auto html = SteamHTMLSurface();
  html->AllowStartRequest(browser, true);
}
void CHTMLSurface::OnJSAlert(HTML_JSAlert_t *pParam) {
  getOut() << "js alert" << std::endl;
}
void CHTMLSurface::OnJSConfirm(HTML_JSConfirm_t *pParam) {
  getOut() << "js confirm" << std::endl;
}
void CHTMLSurface::OnUploadLocalFile(HTML_FileOpenDialog_t *pParam) {
  getOut() << "upload local file" << std::endl;
}
void CHTMLSurface::OnNeedsPaint(HTML_NeedsPaint_t *pParam) {
  getOut() << "needs paint " << pParam->unWide << " " << pParam->unTall << std::endl;

  onPaint(pParam->unWide, pParam->unTall, pParam->pBGRA);
  // vr::VROverlay()->SetOverlayRaw(overlayHandle, (void *)pParam->pBGRA, pParam->unWide, pParam->unTall, 3);
}
void CHTMLSurface::OnNewWindow(HTML_NewWindow_t *pParam) {
  getOut() << "new window" << std::endl;
}
void CHTMLSurface::OnURLChanged(HTML_URLChanged_t *pParam) {
  getOut() << "url changed " << pParam->pchURL << std::endl;
}
void CHTMLSurface::OnBrowserRestarted(HTML_BrowserRestarted_t *pParam) {
  getOut() << "browser restarted" << std::endl;
  if (pParam->unOldBrowserHandle == browser) {
    HTML_BrowserReady_t ready;
    ready.unBrowserHandle = pParam->unBrowserHandle;
    OnBrowserReady(&ready, false);
  }
}
std::unique_ptr<CHTMLSurface> htmlSurface;

// OS specific macros for the example main entry points
// int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
int main(int argc, char **argv) {
  tools::initLogs();
  
  getOut() << "Start" << std::endl;
  
  auto steamRestartOk = SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid);
  getOut() << "steam restart ok " << (void *)steamRestartOk << std::endl;
  auto steamInitOk = SteamAPI_Init();
  getOut() << "steam init ok " << (void *)steamInitOk << std::endl;
  
  std::unique_ptr<CAardvarkCefApp> app(new CAardvarkCefApp());
  /* std::thread renderThread([&]() -> void {
    while (!app->wantsToQuit()) {
      app->runFrame();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }); */

  {  
    char cwdBuf[MAX_PATH];
    if (!GetCurrentDirectory(sizeof(cwdBuf), cwdBuf)) {
      getOut() << "failed to get current directory" << std::endl;
      abort();
    }
    dllDir = cwdBuf;
    dllDir += "\\";
    {
      std::string manifestTemplateFilePath = std::filesystem::weakly_canonical(std::filesystem::path(dllDir + std::string(R"EOF(..\..\..\avrenderer\native-manifest-template.json)EOF"))).string();
      std::string manifestFilePath = std::filesystem::weakly_canonical(std::filesystem::path(dllDir + std::string(R"EOF(\native-manifest.json)EOF"))).string();

      std::string s;
      {
        std::ifstream inFile(manifestTemplateFilePath);
        s = std::string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
      }
      {
        json j = json::parse(s);
        j["path"] = std::filesystem::weakly_canonical(std::filesystem::path(dllDir + std::string(R"EOF(\avrenderer.exe)EOF"))).string();
        s = j.dump(2);
      }
      {    
        std::ofstream outFile(manifestFilePath);
        outFile << s;
        outFile.close();
      }
      
      HKEY hKey;
      LPCTSTR sk = R"EOF(Software\Google\Chrome\NativeMessagingHosts\com.exokit.xrchrome)EOF";
      LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS , &hKey);
      if (openRes == ERROR_FILE_NOT_FOUND) {
        openRes = RegCreateKeyExA(HKEY_CURRENT_USER, sk, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
        
        if (openRes != ERROR_SUCCESS) {
          getOut() << "failed to create registry key: " << (void*)openRes << std::endl;
          abort();
        }
      } else if (openRes != ERROR_SUCCESS) {
        getOut() << "failed to open registry key: " << (void*)openRes << std::endl;
        abort();
      }

      LPCTSTR value = "";
      LPCTSTR data = manifestFilePath.c_str();
      LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)data, strlen(data)+1);
      if (setRes != ERROR_SUCCESS) {
        getOut() << "failed to set registry key: " << (void*)setRes << std::endl;
        abort();
      }

      LONG closeRes = RegCloseKey(hKey);
      if (closeRes != ERROR_SUCCESS) {
        getOut() << "failed to close registry key: " << (void*)closeRes << std::endl;
        abort();
      }
    }
  }
  {  
    app->startRenderer();
    Sleep(2000);
    {
      std::string name("objectTest1");
      std::vector<char> data = readFile("data/avatar.glb");
      auto model = app->renderer->m_renderer->loadModelInstance(name, std::move(data));
      std::vector<float> boneTexture(128*16);
      glm::mat4 jointMat = glm::translate(glm::mat4{1}, glm::vec3(0, 0.2, 0));
      for (size_t i = 0; i < boneTexture.size(); i += 16) {
        memcpy(&boneTexture[i], &jointMat, sizeof(float)*16);
      }
      app->renderer->m_renderer->setBoneTexture(model.get(), boneTexture);
      app->renderer->m_renderer->addToRenderList(model.release());
    }
    {
      std::string name("objectTest2");
      std::vector<float> positions{
        -0.1, 0.5, 0,
        0.1, 0.5, 0,
        -0.1, -0.5, 0,
        0.1, -0.5, 0,
      };
      std::vector<float> normals{
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
      };
      std::vector<float> colors{
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
      };
      std::vector<float> uvs{
        0, 1,
        1, 1,
        0, 0,
        1, 0,
      };
      std::vector<uint16_t> indices{
        0, 2, 1,
        2, 3, 1,
      };
      auto model = app->renderer->m_renderer->createDefaultModelInstance(name);
      model = app->renderer->m_renderer->setModelGeometry(std::move(model), positions, normals, colors, uvs, indices);
      /* std::vector<unsigned char> image = {
        255,
        0,
        0,
        255,
      };
      model = app->renderer->m_renderer->setModelTexture(std::move(model), 1, 1, std::move(image)); */
      auto modelPtr = model.release();
      app->renderer->m_renderer->addToRenderList(modelPtr);
      htmlSurface.reset(new CHTMLSurface([&app, modelPtr](uint32_t width, uint32_t height, const char *data) {
        app->renderer->m_renderer->removeFromRenderList(modelPtr);

        std::vector<unsigned char> image(width * height * 4);
        memcpy(image.data(), data, image.size());
        auto model = app->renderer->m_renderer->setModelTexture(std::unique_ptr(modelPtr), width, height, std::move(image));
        modelPtr = model.release();
        app->renderer->m_renderer->addToRenderList(modelPtr);
      }));
    }
  }
  getOut() << "part 2" << std::endl;
  size_t ids = 0;
  std::map<std::string, std::unique_ptr<IModelInstance>> models;
  {
    setmode(fileno(stdout), O_BINARY);
    setmode(fileno(stdin), O_BINARY);

    freopen(NULL, "rb", stdin);
    freopen(NULL, "wb", stdout);

    char cwdBuf[MAX_PATH];
    if (!GetCurrentDirectory(
      sizeof(cwdBuf),
      cwdBuf
    )) {
      getOut() << "failed to get current directory" << std::endl;
      abort();
    }

    getOut() << "start native host" << std::endl;
    for (;;) {
      uint32_t size;
      std::cin.read((char *)&size, sizeof(uint32_t));
      if (std::cin.good()) {
        std::vector<uint8_t> readbuf(size);
        std::cin.read((char *)readbuf.data(), readbuf.size());
        if (std::cin.good()) {
          json req = json::parse(readbuf);
          json method;
          json args;
          for (json::iterator it = req.begin(); it != req.end(); ++it) {
            if (it.key() == "method") {
              method = it.value();
            } else if (it.key() == "args") {
              args = it.value();
            }
          }
          
          if (method.is_string() && args.is_array()) {
            const std::string methodString = method.get<std::string>();
            getOut() << "method: " << methodString << std::endl;

            /* int i = 0;
            for (json::iterator it = args.begin(); it != args.end(); ++it) {
              const std::string argString = it->get<std::string>();
              std::cout << "arg " << i << ": " << argString << std::endl;
              i++;
            } */
            if (methodString == "startRenderer") {
              app->startRenderer();

              json result = {
                // {"processId", processId}
              };
              json res = {
                {"error", nullptr},
                {"result", result}
              };
              respond(res);
            /* } else if (
              methodString == "addModel" &&
              args.size() >= 1 &&
              args[0].is_string()
            ) {
              std::vector<unsigned char> data = Base64::Decode<unsigned char>(args[0].get<std::string>());

              models[name] = app->renderer->m_renderer->createDefaultModelInstance(name);
              app->renderer->m_renderer->addToRenderList(models[name].get());
              // std::shared_ptr<vkglTF::Model> VulkanExample::findOrLoadModel( std::string modelUri, std::string *psError)
              
              json result = {
                {"id", name}
              };
              json res = {
                {"error", nullptr},
                {"result", result}
              };
              respond(res); */
            } else if (
              methodString == "addObject" &&
              args.size() >= 5 &&
              args[0].is_string() &&
              args[1].is_string() &&
              args[2].is_string() &&
              args[3].is_string() &&
              args[4].is_string()
            ) {
              std::vector<float> positions = Base64::Decode<float>(args[0].get<std::string>());
              std::vector<float> normals = Base64::Decode<float>(args[1].get<std::string>());
              std::vector<float> colors = Base64::Decode<float>(args[2].get<std::string>());
              std::vector<float> uvs = Base64::Decode<float>(args[3].get<std::string>());
              std::vector<uint16_t> indices = Base64::Decode<uint16_t>(args[4].get<std::string>());

              std::string name("object");
              name += std::to_string(++ids);

              models[name] = app->renderer->m_renderer->createDefaultModelInstance(name);
              app->renderer->m_renderer->addToRenderList(models[name].get());
              // std::shared_ptr<vkglTF::Model> VulkanExample::findOrLoadModel( std::string modelUri, std::string *psError)
              
              json result = {
                {"id", name}
              };
              json res = {
                {"error", nullptr},
                {"result", result}
              };
              respond(res);
            } else if (
              methodString == "updateObjectTransform" &&
              args.size() >= 4 &&
              args[0].is_string() &&
              args[1].is_string() &&
              args[2].is_string() &&
              args[3].is_string()
            ) {
              std::string name = args[0].get<std::string>();
              std::vector<float> position = Base64::Decode<float>(args[1].get<std::string>());
              std::vector<float> quaternion = Base64::Decode<float>(args[2].get<std::string>());
              std::vector<float> scale = Base64::Decode<float>(args[3].get<std::string>());

              auto model = models[name].get();
              app->renderer->m_renderer->setModelTransform(models[name].get(), position, quaternion, scale);
              // XXX update geometry
              
              json result = {
                // {"processId", processId}
              };
              json res = {
                {"error", nullptr},
                {"result", result}
              };
              respond(res);
            } else if (
              methodString == "updateObjectGeometry" &&
              args.size() >= 6 &&
              args[0].is_string() &&
              args[1].is_string() &&
              args[2].is_string() &&
              args[3].is_string() &&
              args[4].is_string() &&
              args[5].is_string()
            ) {
              std::string name = args[0].get<std::string>();
              std::vector<float> positions = Base64::Decode<float>(args[1].get<std::string>());
              std::vector<float> normals = Base64::Decode<float>(args[2].get<std::string>());
              std::vector<float> colors = Base64::Decode<float>(args[3].get<std::string>());
              std::vector<float> uvs = Base64::Decode<float>(args[4].get<std::string>());
              std::vector<uint16_t> indices = Base64::Decode<uint16_t>(args[5].get<std::string>());

              models[name] = app->renderer->m_renderer->setModelGeometry(std::move(models[name]), positions, normals, colors, uvs, indices);
              
              json result = {
                // {"processId", processId}
              };
              json res = {
                {"error", nullptr},
                {"result", result}
              };
              respond(res);
            } else if (
              methodString == "updateObjectTexture" &&
              args.size() >= 4 &&
              args[0].is_string() &&
              args[1].is_number() &&
              args[2].is_number() &&
              args[3].is_string()
            ) {
              std::string name = args[0].get<std::string>();
              int width = args[1].get<int>();
              int height = args[2].get<int>();
              std::vector<uint8_t> data = Base64::Decode<uint8_t>(args[3].get<std::string>());

              models[name] = app->renderer->m_renderer->setModelTexture(std::move(models[name]), width, height, std::move(data));
              
              json result = {
                // {"processId", processId}
              };
              json res = {
                {"error", nullptr},
                {"result", result}
              };
              respond(res);
            }
          }
        }
      }
    }
  }

	return 0;
}
