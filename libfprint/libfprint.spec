#
# spec file for package libfprint
#
# Copyright (c) 2012 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


BuildRequires:  ImageMagick-devel glib2-devel libopenssl-devel libusb-devel pkgconfig

Name:           libfprint
Url:            http://reactivated.net/fprint
License:        LGPL v2.1 only
Group:          Development/Libraries/C and C++
PreReq:         %fillup_prereq
Version:        0.0.6
Release:        18.20.1
Summary:        Library for fingerprint reader support
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       glib2 ImageMagick
Source0:        %{name}-%{version}.tar.bz2
Source99:       baselibs.conf
Patch0:         libfprint-gcc-g++-thoenig-01.patch
Patch1:         libfprint-hal-fdi.patch
Patch2:         libfprint-deinit-01.patch
Patch3:         libfprint-validity.patch

%description
The fprint project aims to plug a gap in the Linux desktop: support for
consumer fingerprint reader devices. The fprint project aims to plug a
gap in the Linux desktop: support for consumer fingerprint reader
devices.



%package -n libfprint0
License:        LGPL v2.1 only
Summary:        Library for fingerprint reader support
Group:          Development/Libraries/C and C++
Provides:       %{name} = %{version}
Obsoletes:      libthinkfinger <= 0.3
Provides:       libthinkfinger = 0.3

%description -n libfprint0
The fprint project aims to plug a gap in the Linux desktop: support for
consumer fingerprint reader devices. The fprint project aims to plug a
gap in the Linux desktop: support for consumer fingerprint reader
devices.



%package devel
License:        LGPL v2.1 only
Summary:        Library for fingerprint reader support (developer files)
Requires:       libfprint = %{version} libfprint0 = %{version} glibc-devel libusb-devel glib2-devel ImageMagick-devel libopenssl-devel zlib-devel
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}

%description devel
This package contains the header files, static libraries and
development documentation for libfprint. If you like to develop
programs using libfprint, you will need to install this package.



%prep
%setup
%patch0 -p0
%patch1 -p0
%patch2 -p0
%patch3 -p1

%build
autoreconf -fi
./configure CFLAGS="$RPM_OPT_FLAGS"                             \
            --enable-static=no                                  \
            --prefix=%{_prefix}                                 \
            --libdir=%{_libdir}                                 \
            --mandir=%{_mandir}                                 \
            --includedir=%{_includedir}
make

%install
make install DESTDIR=$RPM_BUILD_ROOT

%post -n libfprint0 -p /sbin/ldconfig

%postun -n libfprint0 -p /sbin/ldconfig

%files -n libfprint0
%defattr(-, root, root)
%dir %{_datadir}/hal
%dir %{_datadir}/hal/fdi
%dir %{_datadir}/hal/fdi/information
%dir %{_datadir}/hal/fdi/information/20thirdparty
%{_libdir}/%{name}.so.*
%{_datadir}/hal/fdi/information/20thirdparty/10-fingerprint-reader-fprint.fdi

%files devel
%defattr(-, root, root)
%dir %{_includedir}/%{name}
%{_includedir}/%{name}/fprint.h
%{_libdir}/%{name}.la
%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/%{name}.pc

%changelog
* Tue Oct 11 2011 tiwai@suse.de
- Add ID for VFS491 device [138a:003d] (bnc#719202)
* Sat Aug 13 2011 dmueller@suse.de
- add baselibs.conf to sources
* Tue Jun 14 2011 thoenig@suse.de
- Update libfprint-validity.patch (bnc#672021)
* Mon May  9 2011 thoenig@suse.de
- Update libfprint-validity.patch adding proper protoypes for 64bit
  (bnc#688310)
* Mon Feb 28 2011 tiwai@suse.de
- Extend xmessage timeout value for Validity enroll (bnc#675167)
* Tue Feb 15 2011 tiwai@suse.de
- Updated libfprint-validity.patch to sync with the device
  initialization in dev_init (bnc#671425)
- Removed libfprint-validity-fix-abort.patch as fixed in the
  validity patch itself
* Tue Feb  8 2011 tiwai@suse.de
- Updated the patch from Validity for improving the enroll
  (multiple scans) (bnc#644149)
- Fixed validty patch not to abort; also check $DISPLAY for popup
* Tue Jan 27 2009 thoenig@suse.de
- add libfprint-deinit-01.patch: Add new function fp_dev_reset used
  to settle the USB device if it was driven into a undefined state.
  (bnc#463557)
* Wed Nov  5 2008 thoenig@suse.de
- move supplementing USB IDs from libfprint0 to pam_fp (bnc#441754)
* Tue Oct 28 2008 thoenig@suse.de
- libfprint0 provide/obsoletes libthinkfinger
* Thu Oct 23 2008 thoenig@suse.de
- generate HAL fdi so that fingerprint readers are properly
  recognized (bnc#438187)
* Mon Oct 13 2008 thoenig@suse.de
- move 'supplements' and 'recommends' to libfprint0
* Thu Oct  9 2008 thoenig@suse.de
- use uppercase for hexadecimal numbers in USB vendor and product
  IDs
* Mon Oct  6 2008 thoenig@suse.de
- add supplements for supported USB devices
- add baselibs.conf (libfprint0) as pam_fp requires libfprint
- fix header for inclusions in C++
* Fri Sep  5 2008 thoenig@suse.de
- initial check-in
