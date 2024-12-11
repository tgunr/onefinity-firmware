import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";
import { resolve } from "path";

// https://vitejs.dev/config/
export default defineConfig({
    plugins: [
        svelte()
    ],
    resolve: {
        alias: {
            $lib: resolve("./src/lib"),
            $dialogs: resolve("./src/dialogs"),
            $components: resolve("./src/components")
        }
    },
    build: {
        target: "chrome60",
        outDir: "dist",
        emptyOutDir: true,
        rollupOptions: {
            input: {
                main: resolve(__dirname, "index.html")
            }
        }
    }
});
