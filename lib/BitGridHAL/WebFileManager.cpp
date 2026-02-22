#include "WebFileManager.h"

#include <Log.h>

namespace BitGrid {
namespace HAL {

static const char* TAG = "Web";

// HTML page with Bootstrap + Vue 3 via CDN
static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>BitGrid File Manager</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body>
    <div id="app" class="container py-4">
        <div class="d-flex justify-content-between align-items-center mb-4">
            <h1>BitGrid SD Card Manager</h1>
            <button @click="reloadPlaylist" class="btn btn-warning" :disabled="reloading">
                <span v-if="reloading" class="spinner-border spinner-border-sm me-2"></span>
                {{ reloading ? 'Reloading...' : 'Reload Playlist' }}
            </button>
        </div>
        
        <!-- Current path breadcrumb -->
        <nav aria-label="breadcrumb">
            <ol class="breadcrumb">
                <li class="breadcrumb-item">
                    <a href="#" @click.prevent="navigateTo('/')">Root</a>
                </li>
                <li v-for="(part, idx) in pathParts" :key="idx" class="breadcrumb-item" :class="{active: idx === pathParts.length - 1}">
                    <span v-if="idx === pathParts.length - 1">{{ part }}</span>
                    <a v-else href="#" @click.prevent="navigateTo(getPathUpTo(idx))">{{ part }}</a>
                </li>
            </ol>
        </nav>

        <!-- File upload and folder creation -->
        <div class="row mb-4">
            <div class="col-md-6">
                <div class="card h-100">
                    <div class="card-body">
                        <h5 class="card-title">Upload File</h5>
                        <form @submit.prevent="uploadFile">
                            <div class="mb-3">
                                <input type="file" class="form-control" ref="fileInput" required>
                            </div>
                            <button type="submit" class="btn btn-primary" :disabled="uploading">
                                <span v-if="uploading" class="spinner-border spinner-border-sm me-2"></span>
                                {{ uploading ? 'Uploading...' : 'Upload' }}
                            </button>
                        </form>
                    </div>
                </div>
            </div>
            <div class="col-md-6">
                <div class="card h-100">
                    <div class="card-body">
                        <h5 class="card-title">Create Folder</h5>
                        <form @submit.prevent="createFolder">
                            <div class="mb-3">
                                <input type="text" class="form-control" v-model="newFolderName" placeholder="Folder name" required>
                            </div>
                            <button type="submit" class="btn btn-success" :disabled="creating">
                                <span v-if="creating" class="spinner-border spinner-border-sm me-2"></span>
                                {{ creating ? 'Creating...' : 'Create Folder' }}
                            </button>
                        </form>
                    </div>
                </div>
            </div>
        </div>

        <!-- File list -->
        <div class="card">
            <div class="card-body">
                <h5 class="card-title">Files in {{ currentPath || '/' }}</h5>
                <div v-if="loading" class="text-center py-5">
                    <div class="spinner-border" role="status">
                        <span class="visually-hidden">Loading...</span>
                    </div>
                </div>
                <div v-else-if="error" class="alert alert-danger">{{ error }}</div>
                <ul v-else class="list-group">
                    <li v-if="currentPath" class="list-group-item">
                        <a href="#" @click.prevent="navigateUp" class="text-decoration-none">
                            📁 <strong>..</strong> (Parent Directory)
                        </a>
                    </li>
                    <li v-for="item in items" :key="item.name" class="list-group-item d-flex justify-content-between align-items-center">
                        <div>
                            <a v-if="item.isDir" href="#" @click.prevent="navigateTo(item.path)" class="text-decoration-none">
                                📁 <strong>{{ item.name }}</strong>
                            </a>
                            <span v-else>📄 {{ item.name }} <small class="text-muted">({{ formatSize(item.size) }})</small></span>
                        </div>
                        <button v-if="!item.isDir" @click="deleteFile(item.path)" class="btn btn-sm btn-danger">Delete</button>
                    </li>
                    <li v-if="items.length === 0" class="list-group-item text-muted">Empty directory</li>
                </ul>
            </div>
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/vue@3.3.4/dist/vue.global.prod.js"></script>
    <script>
        const { createApp } = Vue;

        createApp({
            data() {
                return {
                    currentPath: '',
                    items: [],
                    loading: false,
                    uploading: false,
                    creating: false,
                    reloading: false,
                    newFolderName: '',
                    error: null
                };
            },
            computed: {
                pathParts() {
                    return this.currentPath ? this.currentPath.split('/').filter(p => p) : [];
                }
            },
            methods: {
                async loadDirectory(path) {
                    this.loading = true;
                    this.error = null;
                    try {
                        const response = await fetch(`/list?path=${encodeURIComponent(path || '/')}`);
                        if (!response.ok) throw new Error('Failed to load directory');
                        const data = await response.json();
                        this.items = data.items || [];
                        this.currentPath = path;
                    } catch (e) {
                        this.error = e.message;
                    } finally {
                        this.loading = false;
                    }
                },
                navigateTo(path) {
                    this.loadDirectory(path);
                },
                navigateUp() {
                    const parts = this.currentPath.split('/').filter(p => p);
                    parts.pop();
                    this.loadDirectory(parts.join('/'));
                },
                getPathUpTo(idx) {
                    return this.pathParts.slice(0, idx + 1).join('/');
                },
                async uploadFile() {
                    const file = this.$refs.fileInput.files[0];
                    if (!file) return;

                    this.uploading = true;
                    const formData = new FormData();
                    formData.append('file', file);

                    try {
                        const path = encodeURIComponent(this.currentPath || '/');
                        const response = await fetch(`/upload?path=${path}`, {
                            method: 'POST',
                            body: formData
                        });
                        if (!response.ok) throw new Error('Upload failed');
                        this.$refs.fileInput.value = '';
                        await this.loadDirectory(this.currentPath);
                    } catch (e) {
                        alert('Upload failed: ' + e.message);
                    } finally {
                        this.uploading = false;
                    }
                },
                async deleteFile(path) {
                    if (!confirm(`Delete ${path}?`)) return;
                    
                    try {
                        const response = await fetch(`/delete?path=${encodeURIComponent(path)}`, {
                            method: 'DELETE'
                        });
                        if (!response.ok) throw new Error('Delete failed');
                        await this.loadDirectory(this.currentPath);
                    } catch (e) {
                        alert('Delete failed: ' + e.message);
                    }
                },
                async createFolder() {
                    if (!this.newFolderName.trim()) return;
                    
                    this.creating = true;
                    try {
                        const response = await fetch('/mkdir', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({
                                path: this.currentPath || '/',
                                name: this.newFolderName
                            })
                        });
                        if (!response.ok) throw new Error('Failed to create folder');
                        this.newFolderName = '';
                        await this.loadDirectory(this.currentPath);
                    } catch (e) {
                        alert('Create folder failed: ' + e.message);
                    } finally {
                        this.creating = false;
                    }
                },
                async reloadPlaylist() {
                    this.reloading = true;
                    try {
                        const response = await fetch('/reload', {
                            method: 'POST'
                        });
                        if (!response.ok) throw new Error('Reload failed');
                        alert('Playlist reloaded successfully!');
                    } catch (e) {
                        alert('Reload failed: ' + e.message);
                    } finally {
                        this.reloading = false;
                    }
                },
                formatSize(bytes) {
                    if (bytes < 1024) return bytes + ' B';
                    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
                    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
                }
            },
            mounted() {
                this.loadDirectory('');
            }
        }).mount('#app');
    </script>
</body>
</html>
)rawliteral";

WebFileManager::WebFileManager(uint16_t port) : server_(port) {}

bool WebFileManager::begin(ReloadCallback reloadCb) {
    reloadCallback_ = reloadCb;
    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/list", HTTP_GET, [this]() { handleList(); });
    server_.on("/upload", HTTP_POST, 
        [this]() { handleUploadComplete(); },
        [this]() { handleUpload(); }
    );
    server_.on("/delete", HTTP_DELETE, [this]() { handleDelete(); });
    server_.on("/mkdir", HTTP_POST, [this]() { handleCreateFolder(); });
    server_.on("/reload", HTTP_POST, [this]() { handleReloadPlaylist(); });
    server_.onNotFound([this]() { handleNotFound(); });

    server_.begin();
    Log::info(TAG, "Web server started on port 80");
    return true;
}

void WebFileManager::tick() {
    server_.handleClient();
}

void WebFileManager::handleRoot() {
    server_.send_P(200, "text/html", HTML_PAGE);
}

void WebFileManager::handleList() {
    String path = server_.arg("path");
    if (path.isEmpty()) path = "/";

    String json = "[";
    File dir = SD.open(path);
    
    if (!dir || !dir.isDirectory()) {
        server_.send(500, "application/json", "{\"error\":\"Invalid directory\"}");
        return;
    }

    bool first = true;
    File file = dir.openNextFile();
    while (file) {
        if (!first) json += ",";
        first = false;

        String name = String(file.name());
        // Strip leading path if present
        int lastSlash = name.lastIndexOf('/');
        if (lastSlash >= 0) {
            name = name.substring(lastSlash + 1);
        }

        String fullPath = path;
        if (!fullPath.endsWith("/")) fullPath += "/";
        fullPath += name;

        json += "{";
        json += "\"name\":\"" + name + "\",";
        json += "\"path\":\"" + fullPath + "\",";
        json += "\"isDir\":" + String(file.isDirectory() ? "true" : "false") + ",";
        json += "\"size\":" + String(file.size());
        json += "}";

        file = dir.openNextFile();
    }
    json += "]";

    server_.send(200, "application/json", "{\"items\":" + json + "}");
}

void WebFileManager::handleUpload() {
    HTTPUpload& upload = server_.upload();
    static File uploadFile;

    if (upload.status == UPLOAD_FILE_START) {
        String path = server_.arg("path");
        if (path.isEmpty()) path = "/";
        if (!path.endsWith("/")) path += "/";
        
        String filename = path + String(upload.filename);
        Log::info(TAG, "Upload start: %s", filename.c_str());
        
        uploadFile = SD.open(filename, FILE_WRITE);
        if (!uploadFile) {
            Log::error(TAG, "Failed to open file for writing: %s", filename.c_str());
        }
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
            uploadFile.write(upload.buf, upload.currentSize);
        }
    } 
    else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
            Log::info(TAG, "Upload complete: %u bytes", upload.totalSize);
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (uploadFile) {
            uploadFile.close();
        }
        Log::error(TAG, "Upload aborted");
    }
}

void WebFileManager::handleUploadComplete() {
    server_.send(200, "application/json", "{\"success\":true}");
}

void WebFileManager::handleCreateFolder() {
    // Parse JSON body
    String body = server_.arg("plain");
    
    // Simple JSON parsing (looking for "path" and "name")
    int pathIdx = body.indexOf("\"path\":");
    int nameIdx = body.indexOf("\"name\":");
    
    if (pathIdx < 0 || nameIdx < 0) {
        server_.send(400, "application/json", "{\"error\":\"Invalid request\"}");
        return;
    }
    
    // Extract path value
    int pathStart = body.indexOf('"', pathIdx + 7) + 1;
    int pathEnd = body.indexOf('"', pathStart);
    String path = body.substring(pathStart, pathEnd);
    
    // Extract name value
    int nameStart = body.indexOf('"', nameIdx + 7) + 1;
    int nameEnd = body.indexOf('"', nameStart);
    String name = body.substring(nameStart, nameEnd);
    
    if (name.isEmpty()) {
        server_.send(400, "application/json", "{\"error\":\"Name cannot be empty\"}");
        return;
    }
    
    if (!path.endsWith("/")) path += "/";
    String fullPath = path + name;
    
    if (SD.mkdir(fullPath)) {
        Log::info(TAG, "Created folder: %s", fullPath.c_str());
        server_.send(200, "application/json", "{\"success\":true}");
    } else {
        Log::error(TAG, "Failed to create folder: %s", fullPath.c_str());
        server_.send(500, "application/json", "{\"error\":\"Failed to create folder\"}");
    }
}

void WebFileManager::handleDelete() {
    String path = server_.arg("path");
    if (path.isEmpty()) {
        server_.send(400, "application/json", "{\"error\":\"No path specified\"}");
        return;
    }

    if (SD.remove(path)) {
        Log::info(TAG, "Deleted: %s", path.c_str());
        server_.send(200, "application/json", "{\"success\":true}");
    } else {
        Log::error(TAG, "Failed to delete: %s", path.c_str());
        server_.send(500, "application/json", "{\"error\":\"Delete failed\"}");
    }
}

void WebFileManager::handleReloadPlaylist() {
    if (!reloadCallback_) {
        server_.send(500, "application/json", "{\"error\":\"Reload callback not set\"}");
        return;
    }
    
    reloadCallback_();
    server_.send(200, "application/json", "{\"success\":true,\"message\":\"Playlist reloaded\"}");
}

void WebFileManager::handleNotFound() {
    server_.send(404, "text/plain", "Not found");
}

} // namespace HAL
} // namespace BitGrid
