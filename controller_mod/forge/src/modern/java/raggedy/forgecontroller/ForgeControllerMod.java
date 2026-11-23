package raggedy.forgecontroller;

import raggedy.neoforgecontroller.NeoForgeControllerCompat;
import raggedy.neoforgecontroller.NeoForgeControllerSettings;
import net.minecraftforge.fml.common.Mod;

@Mod(ForgeControllerMod.MOD_ID)
public final class ForgeControllerMod {
    public static final String MOD_ID = "raggedy_forge_controller";

    public ForgeControllerMod() {
        NeoForgeControllerSettings.load();
        NeoForgeControllerCompat.ensureInitialized();
    }
}
