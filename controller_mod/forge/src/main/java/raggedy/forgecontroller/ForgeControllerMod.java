package raggedy.forgecontroller;

import net.minecraftforge.fml.common.Mod;

@Mod(ForgeControllerMod.MOD_ID)
public final class ForgeControllerMod {
    public static final String MOD_ID = "raggedy_forge_controller";

    public ForgeControllerMod() {
        ForgeControllerLog.log("Raggedy Forge controller mod loaded (1.0.4)");
        ForgeControllerSettings.load();
        ForgeControllerCompat.ensureInitialized();
    }
}
