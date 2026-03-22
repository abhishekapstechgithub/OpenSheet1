Name:           opensheet
Version:        1.0.0
Release:        1%{?dist}
Summary:        Professional open-source spreadsheet application
License:        MIT
URL:            https://github.com/opensheet/opensheet
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++ >= 12
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtcharts-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  sqlite-devel
BuildRequires:  python3-devel
BuildRequires:  ninja-build
BuildRequires:  libGL-devel

Requires:       qt6-qtbase
Requires:       qt6-qtcharts
Requires:       qt6-qtsvg
Requires:       sqlite-libs
Requires:       hicolor-icon-theme

%description
OpenSheet is a professional, open-source spreadsheet application built with
C++20 and Qt6. It supports Excel-compatible formulas, multiple file formats
(.opensheet, .xlsx, .csv), charts, conditional formatting, and a plugin system.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DOPENSHEET_BUILD_TESTS=OFF \
    -DOPENSHEET_PYTHON_SUPPORT=ON
%cmake_build

%install
%cmake_install
install -Dm644 resources/icons/logo.png \
    %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/opensheet.png
install -Dm644 installer/linux/opensheet.desktop \
    %{buildroot}%{_datadir}/applications/opensheet.desktop
install -Dm644 installer/linux/io.opensheet.OpenSheet.metainfo.xml \
    %{buildroot}%{_datadir}/metainfo/io.opensheet.OpenSheet.metainfo.xml

for dir in themes localization addons config; do
    cp -r $dir %{buildroot}%{_datadir}/opensheet/
done

%files
%license LICENSE
%doc README.md docs/ARCHITECTURE.md
%{_bindir}/opensheet
%{_datadir}/opensheet/
%{_datadir}/applications/opensheet.desktop
%{_datadir}/icons/hicolor/256x256/apps/opensheet.png
%{_datadir}/metainfo/io.opensheet.OpenSheet.metainfo.xml

%changelog
* Sat Mar 22 2025 OpenSheet Contributors <dev@opensheet.io> - 1.0.0-1
- Initial release
