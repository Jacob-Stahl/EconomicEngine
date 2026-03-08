# Economic Engine Library (EELib)

## WebAssembly Build Setup

To compile the project for WebAssembly, you need to install the Emscripten SDK (emsdk).

### macOS Installation

1.  **Clone the repository**
    ```bash
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ```

2.  **Install the latest version**
    ```bash
    ./emsdk install latest
    ```

3.  **Activate the latest version**
    ```bash
    ./emsdk activate latest
    ```

4.  **Set up environment variables**
    Run this in your terminal (or add it to your `.zshrc` / `.bash_profile` for permanence):
    ```bash
    source ./emsdk_env.sh
    ```

5.  **Verify installation**
    ```bash
    emcc --version
    ```

Once installed, the `emcc` compiler will locate `<emscripten/bind.h>` automatically.

### VS Code IntelliSense Configuration (Optional)
If you are using VS Code and want IntelliSense to recognize Emscripten headers (like `<emscripten/bind.h>`), you can add the include path to your `.vscode/c_cpp_properties.json` file.

Add the path to the `emscripten/system/include` directory, typically found inside your `emsdk` installation:

```json
"includePath": [
    "${workspaceFolder}/**",
    "/path/to/emsdk/upstream/emscripten/system/include"
]
```

## Building and Running the Web Demo

### 1. Build the WASM Module
We have provided a script to compile the C++ code into a WebAssembly module.

```bash
# Make the script executable (only needed once)
chmod +x build_wasm.sh

# Run the build script
./build_wasm.sh
```
This will generate `eelib.js` and `eelib.wasm` in the `webdemo/` folder.

### 2. Run the Local Web Server
Due to browser security restrictions (CORS), you cannot open the HTML file directly from the filesystem. You must serve it via a local web server.

```bash
cd webdemo
python3 -m http.server
```

### 3. View the Demo
Open your browser and navigate to:
[http://localhost:8000](http://localhost:8000)

Check the browser console (Right Click -> Inspect -> Console) to see the output.

## Deploying the Web Demo to GitHub Pages

This repository includes a GitHub Actions workflow at [.github/workflows/deploy-pages.yml](.github/workflows/deploy-pages.yml) that builds the WebAssembly demo and publishes the contents of [webdemo](webdemo) to GitHub Pages.

### One-time GitHub setup

In your repository settings:

1. Open `Settings` -> `Pages`
2. Under `Build and deployment`, set `Source` to `GitHub Actions`

### Deployment behavior

- Pushes to `main` or `master` that touch the WASM build, the `eelib` sources, or the demo page will trigger a deployment.
- You can also trigger the workflow manually from the `Actions` tab.
