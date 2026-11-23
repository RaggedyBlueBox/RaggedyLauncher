package raggedy.xboxcompat;

public final class ReturnToLauncherSignal extends RuntimeException {
    public static final String MARKER = "RaggedyReturnToLauncher";

    public ReturnToLauncherSignal() {
        super(MARKER);
    }

    @Override
    public synchronized Throwable fillInStackTrace() {
        return this;
    }
}
