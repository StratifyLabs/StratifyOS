# api

The `api` folder contains built-in Stratify OS api interfaces. Other libraries may install additional api header files to augment the built-in ones.

These api libraries are how Stratify OS allows multiple applications to share libraries.

All api libraries should use the `sos_api_t` as the first entry in the API. See [sos api](sos_api.h) for details.