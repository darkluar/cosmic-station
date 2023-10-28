package emu.zenith.data

import androidx.annotation.Keep
import java.io.FileInputStream

// For some reason, proguard-rules.pro can't keep these fields from KernelModel in release packages
@Keep
data class BiosInfo(
    var position: Int,

    var fileAlive: FileInputStream,
    var selected: Boolean,

    var biosFilename: String,
    var biosName: String,
    var biosDetails: String
)