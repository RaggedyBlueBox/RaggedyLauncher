package net.fabricmc.loader.impl.util;

import java.io.IOException;
import java.net.URI;
import java.nio.file.FileSystem;
import java.nio.file.FileSystemAlreadyExistsException;
import java.nio.file.FileSystemNotFoundException;
import java.nio.file.FileSystems;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public final class FileSystemUtil {
    public static class FileSystemDelegate implements AutoCloseable {
        private final FileSystem fileSystem;
        private final boolean owner;

        public FileSystemDelegate(FileSystem fileSystem, boolean owner) {
            this.fileSystem = fileSystem;
            this.owner = owner;
        }

        public FileSystem get() {
            return fileSystem;
        }

        @Override
        public void close() throws IOException {
            if (owner) {
                fileSystem.close();
            }
        }
    }

    private FileSystemUtil() {
    }

    public enum AccessMode {
        READ_ONLY("accessMode", "readOnly"),
        READ_WRITE(),
        READ_WRITE_CREATE("create", "true");

        final Map<String, String> fsArgs;

        AccessMode(String... args) {
            Map<String, String> argsMap;

            if (args.length == 0) {
                argsMap = Collections.emptyMap();
            } else {
                argsMap = new HashMap<>(args.length / 2);

                for (int i = 0; i < args.length; i += 2) {
                    argsMap.put(args[i], args[i + 1]);
                }
            }

            this.fsArgs = argsMap;
        }
    }

    public static FileSystemDelegate getJarFileSystem(Path path, AccessMode accessMode) throws IOException {
        try {
            FileSystem fs = FileSystems.newFileSystem(path, accessMode.fsArgs);
            return new FileSystemDelegate(fs, true);
        } catch (FileSystemAlreadyExistsException ignored) {
            FileSystem fs;
            try {
                fs = FileSystems.getFileSystem(URI.create("jar:" + path.toUri()));
            } catch (FileSystemNotFoundException ex) {
                throw new IOException("Error accessing " + path + ": " + ex, ex);
            }
            if (accessMode != AccessMode.READ_ONLY && fs.isReadOnly()) {
                throw new IllegalStateException(String.format("file %s is already open read-only and can't be reopened for writing", path));
            }
            return new FileSystemDelegate(fs, false);
        } catch (IOException | RuntimeException ex) {
            throw new IOException("Error accessing " + path + ": " + ex, ex);
        }
    }

    public static FileSystemDelegate getJarFileSystem(URI uri, AccessMode accessMode) throws IOException {
        try {
            return getJarFileSystem(uriToPath(uri), accessMode);
        } catch (IllegalArgumentException ex) {
            throw new IOException("Error accessing " + uri + ": " + ex, ex);
        }
    }

    public static FileSystemDelegate getJarFileSystem(Path path, boolean create) throws IOException {
        return getJarFileSystem(path, create ? AccessMode.READ_WRITE_CREATE : AccessMode.READ_ONLY);
    }

    public static FileSystemDelegate getJarFileSystem(URI uri, boolean create) throws IOException {
        return getJarFileSystem(uri, create ? AccessMode.READ_WRITE_CREATE : AccessMode.READ_ONLY);
    }

    private static Path uriToPath(URI uri) {
        String raw = uri.getRawSchemeSpecificPart();
        if ("jar".equalsIgnoreCase(uri.getScheme()) && raw != null) {
            int separator = raw.indexOf("!/");
            if (separator >= 0) {
                raw = raw.substring(0, separator);
            }
        }
        return Paths.get(URI.create(raw));
    }
}
