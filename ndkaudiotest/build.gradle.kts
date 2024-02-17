import com.android.build.api.dsl.Packaging

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "id.psw.ndkaudiotest"
    compileSdk = 34

    defaultConfig {
        applicationId = "id.psw.ndkaudiotest"
        minSdk = 28
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                cppFlags += ""
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
            packaging {
                resources.excludes.add("**/*.kotlin_builtins")
                resources.excludes.add("**/*.kotlin_module")
                resources.excludes.add("**/kotlin-tooling-metadata.json")
            }
            externalNativeBuild {
                cmake {
                    arguments += "-DCMAKE_BUILD_TYPE:STRING=MinSizeRel"
                    cFlags += "-Os"
                }
            }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}

dependencies {
}