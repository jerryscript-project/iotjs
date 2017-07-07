Name: iotjs
Version: 1.0.0
Release: 0
Summary: Platform for Internet of Things with JavaScript
Group: Network & Connectivity
License: Apache-2.0
URL: https://www.iotjs.net/
Source:     %{name}-%{version}.tar.gz
Source1:    %{name}.pc.in
Source1001: %{name}.manifest

BuildRequires: python
BuildRequires: cmake
BuildRequires: glibc-static
BuildRequires: aul
BuildRequires: pkgconfig(appcore-agent)
BuildRequires: pkgconfig(capi-appfw-service-application)
BuildRequires: pkgconfig(capi-appfw-app-common)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-system-peripheral-io)
BuildRequires: pkgconfig(dlog)
#for https
BuildRequires:  openssl-devel
BuildRequires:  libcurl-devel

Requires(postun): /sbin/ldconfig
Requires(post): /sbin/ldconfig

%description
Platform for Internet of Things with JavaScript

# default is RELEASE mode.
# If DEBUG mode is needed, please use tizen_build_devel_mode
%define RELEASE False
# For Example
%if %{RELEASE} == "True"
%define build_mode release
%else
%define build_mode debug
%endif

# Default values to be eventually overiden BEFORE or as gbs params:
%{!?RELEASE: %define RELEASE 0}

%package service
Summary: Development files for %{name}
Group: Network & Connectivity/Service
Requires: %{name} = %{version}-%{release}

%description service
The %{name}-service package contains service iotjs files for
developing applications that use %{name}.

%package devel
Summary: Header files for %{name}
Group: Network & Connectivity/Service
Requires: %{name} = %{version}-%{release}

%description devel
Development libraries for %{name}

%prep
%setup -q -c
chmod g-w %_sourcedir/*
cat LICENSE
cp %{SOURCE1001} .

%build
./tools/build.py --clean --buildtype=%{build_mode} --target-arch=arm \
 --target-os=tizen --target-board=artik10 \
 --external-shared-lib=capi-system-peripheral-io \
 --compile-flag=-D__TIZEN__ \
 --iotjs-include-module=dgram,gpio,i2c \
 --no-init-submodule --no-parallel-build --no-check-test

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_includedir}/iotjs
mkdir -p %{buildroot}%{_libdir}/iotjs
mkdir -p %{buildroot}%{_libdir}/pkgconfig


cp ./build/arm-tizen/%{build_mode}/bin/iotjs %{buildroot}%{_bindir}/
cp ./build/arm-tizen/%{build_mode}/lib/* %{buildroot}%{_libdir}/iotjs/

cp ./include/*.h %{buildroot}%{_includedir}
cp ./src/*.h %{buildroot}%{_includedir}
cp ./config/tizen/packaging/%{name}.pc.in %{buildroot}/%{_libdir}/pkgconfig/%{name}.pc

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
%manifest config/tizen/packaging/%{name}.manifest
%defattr(-,root,root,-)
%license LICENSE
%{_bindir}/*

%files devel
%defattr(-,root,root,-)
%{_libdir}/iotjs/*.a
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/*
