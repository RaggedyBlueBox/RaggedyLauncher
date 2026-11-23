package raggedy.xboxcompat.mixin;

import raggedy.xboxcompat.XboxCompatLog;
import net.minecraft.class_310;
import net.minecraft.class_437;
import net.minecraft.class_542;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(class_310.class)
public abstract class MinecraftClientProbeMixin {
    private static long raggedy$tickCount = 0L;
    private static boolean raggedy$uncaughtHandlerInstalled = false;
    private static boolean raggedy$playableLogged = false;
    private static boolean raggedy$clientConstructed = false;

    @Shadow
    public class_437 field_1755;

    @Inject(method = "<init>", at = @At("TAIL"))
    private void raggedy$logClientConstructed(class_542 args, CallbackInfo ci) {
        XboxCompatLog.log("MinecraftClient constructed");
        raggedy$clientConstructed = true;
        raggedy$installUncaughtExceptionHandler();
    }

    @Inject(method = "method_1514", at = @At("HEAD"))
    private void raggedy$logMainLoopEntered(CallbackInfo ci) {
        XboxCompatLog.log("MinecraftClient main loop entered");
    }

    @Inject(method = "method_1514", at = @At("TAIL"))
    private void raggedy$logMainLoopExited(CallbackInfo ci) {
        XboxCompatLog.log("MinecraftClient main loop exited");
    }

    @Inject(method = "method_1507", at = @At("HEAD"))
    private void raggedy$logSetScreenHead(class_437 screen, CallbackInfo ci) {
        XboxCompatLog.log("setScreen head -> " + raggedy$screenName(screen));
    }

    @Inject(method = "method_1507", at = @At("TAIL"))
    private void raggedy$logSetScreenTail(class_437 screen, CallbackInfo ci) {
        XboxCompatLog.log("setScreen tail -> current=" + raggedy$screenName(this.field_1755));
        if (raggedy$clientConstructed && this.field_1755 != null) {
            raggedy$logPlayableOnce();
        }
    }

    // intermediary names prevent a title screen type check here
    private static synchronized void raggedy$logPlayableOnce() {
        if (raggedy$playableLogged) {
            return;
        }
        raggedy$playableLogged = true;
        XboxCompatLog.log("raggedy:playable");
    }

    @Inject(method = "method_1574", at = @At("HEAD"))
    private void raggedy$logClientTick(CallbackInfo ci) {
        raggedy$tickCount++;
        if (raggedy$tickCount <= 5 || raggedy$tickCount % 120 == 0) {
            XboxCompatLog.log("client tick=" + raggedy$tickCount
                + " screen=" + raggedy$screenName(this.field_1755));
        }
    }

    private static String raggedy$screenName(class_437 screen) {
        return screen == null ? "null" : screen.getClass().getName();
    }

    private static synchronized void raggedy$installUncaughtExceptionHandler() {
        if (raggedy$uncaughtHandlerInstalled) {
            return;
        }

        Thread.UncaughtExceptionHandler previous = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
            @Override
            public void uncaughtException(Thread thread, Throwable throwable) {
                XboxCompatLog.logException(
                    "Uncaught exception on thread=" + thread.getName(),
                    throwable);
                if (previous != null) {
                    previous.uncaughtException(thread, throwable);
                }
            }
        });
        raggedy$uncaughtHandlerInstalled = true;
        XboxCompatLog.log("Installed default uncaught exception handler");
    }
}
