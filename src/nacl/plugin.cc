// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the NaCl-LICENSE file.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/file_system.h>

#include <SDL_video.h>
extern int wesnoth_main(int argc, char **argv);
#include <SDL.h>
#include <SDL_nacl.h>

#include <nacl-mounts/base/KernelProxy.h>
#include <nacl-mounts/base/MainThreadRunner.h>
#include <nacl-mounts/http2/HTTP2Mount.h>
#include <nacl-mounts/pepper/PepperMount.h>


const char* http_dirs[] = {
#include <dir_list.h>
};

struct http_file_info {
  const char* path;
  size_t size;
} http_files[] = {
#include <file_list.h>
};

struct http_pack_info {
  const char* path;
  const char* pack_path;
  off_t offset;
} http_packs[] = {
#include <pack_list.h>
};


class PluginInstance : public pp::Instance {
 public:
  explicit PluginInstance(PP_Instance instance) : pp::Instance(instance),
                                                  sdl_main_thread_(0),
                                                  width_(0),
                                                  height_(0),
                                                  progress_handler_(this),
                                                  directory_reader_(this) {
    RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
    RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);

    proxy_ = KernelProxy::KPInstance();
    runner_ = new MainThreadRunner(this);

    fprintf(stderr, "Requesting an HTML5 local persistent filesystem.\n");
    fflush(stderr);
    fs_ = new pp::FileSystem(this, PP_FILESYSTEMTYPE_LOCALPERSISTENT);
  }

  ~PluginInstance() {
    if (sdl_main_thread_) {
      pthread_join(sdl_main_thread_, NULL);
    }
  }

  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip) {
    fprintf(stderr, "did change view, new %dx%d, old %dx%d\n",
        position.size().width(), position.size().height(),
        width_, height_);
    fflush(stderr);

    width_ = position.size().width();
    height_ = position.size().height();

    SDL_NACL_SetInstance(pp_instance(), width_, height_);

    if (sdl_thread_started_ == false) {
      // It seems this call to SDL_Init is required. Calling from
      // sdl_main() isn't good enough.
      // Perhaps it must be called from the main thread?
      int lval = SDL_Init(SDL_INIT_AUDIO);
      assert(lval >= 0);
      if (0 == pthread_create(&sdl_main_thread_, NULL, sdl_thread_static, this)) {
        sdl_thread_started_ = true;
      }
    }
  }

  bool HandleInputEvent(const pp::InputEvent& event) {
    SDL_NACL_PushEvent(event);
    return true;
  }

  void HandleMessage(const pp::Var& message) {
    std::string s = message.AsString();
    directory_reader_.HandleResponse(s);
  }

  bool Init(int argc, const char* argn[], const char* argv[]) {
    return true;
  }

 private:
  bool sdl_thread_started_;
  pthread_t sdl_main_thread_;
  int width_;
  int height_;
  KernelProxy* proxy_;
  MainThreadRunner* runner_;
  pp::FileSystem* fs_;

  static void* sdl_thread_static(void* param) {
    return reinterpret_cast<PluginInstance*>(param)->sdl_thread();
  }

  void* sdl_thread() {
    fprintf(stderr, "Initializing nacl-mounts.\n");
    fflush(stderr);

    // Setup writable homedir.
    PepperMount* pepper_mount = new PepperMount(runner_, fs_, 20 * 1024 * 1024);
    pepper_mount->SetDirectoryReader(&directory_reader_);
    pepper_mount->SetPathPrefix("/wesnoth-userdata");

    proxy_->mkdir("/wesnoth-userdata", 0777);
    int res = proxy_->mount("/wesnoth-userdata", pepper_mount);

    // The following lines can be removed when nacl-mounts starts intercepting mkdir() calls.
    proxy_->mkdir("/wesnoth-userdata/saves", 0777);

    // Setup r/o data directory in /usr/local/share/wesnoth
    HTTP2Mount* http2_mount = new HTTP2Mount(runner_, "./usr/local/share/wesnoth");
    http2_mount->SetLocalCache(fs_, 350*1024*1024, "/wesnoth0", true);
    http2_mount->SetProgressHandler(&progress_handler_);
 
    fprintf(stderr, "Registering known files.\n");
    fflush(stderr);
    for (int i = 0; i < sizeof(http_dirs) / sizeof(*http_dirs); ++i) {
      char* path = (char*)http_dirs[i];
      if (path && *path)
        http2_mount->AddDir(path);
    }

    for (int i = 0; i < sizeof(http_files) / sizeof(*http_files); ++i) {
      char* path = (char*)http_files[i].path;
      size_t size = http_files[i].size;
      if (path && *path)
        http2_mount->AddFile(path, size);
    }

    for (int i = 0; i < sizeof(http_packs) / sizeof(*http_packs); ++i) {
      char* path = (char*)http_packs[i].path;
      char* pack_path = (char*)http_packs[i].pack_path;
      off_t offset = http_packs[i].offset;
      if (path && *path) {
        http2_mount->SetInPack(path, pack_path, offset);
      }
    }

    http2_mount->SetInMemory("/fonts/Andagii.ttf", true);
    http2_mount->SetInMemory("/fonts/DejaVuSans.ttf", true);
    http2_mount->SetInMemory("/fonts/wqy-zenhei.ttc", true);

    fprintf(stderr, "Mounting the filesystem.\n");
    fflush(stderr);
    proxy_->mkdir("/usr", 0777);
    proxy_->mkdir("/usr/local", 0777);
    proxy_->mkdir("/usr/local/share", 0777);
    res = proxy_->mount("/usr/local/share/wesnoth", http2_mount);
    if (!res) {
      fprintf(stderr, "FS initialization success.\n");
    } else {
      fprintf(stderr, "FS initialization failure.\n");
    }
    fflush(stderr);

    // Finally, launch the game.
    char res_s[100];
    snprintf(res_s, sizeof(res_s), "%dx%d", width_, height_);
    static char const * argv[] = {"wesnoth", "-r", res_s, NULL};
    printf("starting game thread: %s\n", res_s);
    wesnoth_main(sizeof(argv) / sizeof(*argv) - 1, (char**)argv);
    return NULL;
  }

  class ProgressHandler : public HTTP2ProgressHandler {
  public:
    pp::Instance* instance_;

    ProgressHandler(pp::Instance* instance) : instance_(instance) {}

    void HandleProgress(std::string& path, int64_t bytes, int64_t size) {
      char buf[100];
      snprintf(buf, sizeof(buf), "%llu,%llu", (unsigned long long)bytes,
          (unsigned long long)size);
      std::string message = "[\"" + path + "\"," + buf + "]";
      instance_->PostMessage(message);
    }
  };

  ProgressHandler progress_handler_;

  class JSDirectoryReader: public DirectoryReader {
  public:
    pp::Instance* instance_;
    pp::CompletionCallback cc_;
    std::set<std::string>* entries_;

    JSDirectoryReader(pp::Instance* instance) : instance_(instance) {}

    int ReadDirectory(const std::string& path, std::set<std::string>* entries, const pp::CompletionCallback& cc) {
      cc_ = cc;
      entries_ = entries;
      std::string message = "[\"ReadDirectory\",\"" + path + "\"]";
      instance_->PostMessage(message);
    }

    void HandleResponse(const std::string& response) {
      fprintf(stderr, "response: %s\n", response.c_str());
      std::string::const_iterator ind = response.begin();
      std::string::const_iterator next = response.begin();
      while (ind != response.end() && next != response.end()) {
        if (*next == '\n' && ind != next) {
          if (*ind == '\n') {
            ++ind;
          }
          if (ind != next) {
            entries_->insert(std::string(ind, next));
          }
          ind = next;
        }
        ++next;
      }
      if (ind != next) {
	std::string last(ind, next-1);
        if (!last.empty()) {
          entries_->insert(last);
        }
      }
      cc_.Run(PP_OK);
    }
  };

  JSDirectoryReader directory_reader_;
};

class PepperModule : public pp::Module {
public:
  // Create and return a PluginInstanceInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new PluginInstance(instance);
  }
};

namespace pp {
  Module* CreateModule() {
    return new PepperModule();
  }
}  // namespace pp
