module RestinioOpenSSLFind

  def self.file_exists?( dir, f )
    fname = File.join( dir, f )
    if File.exist?( fname ) and File.file?( fname )
      fname
    else
      nil
    end
  end

  def self.all_files_exist( dir, files )
    not files.find{|f| !file_exists?(dir, f ) }
  end

  def self.find_first_openssl_libs( openssl_before_11, openssl_after_11 )

    lib_dirs = ENV[ "LIB" ]
    if lib_dirs

      dirs_to_look = lib_dirs.split( ';' ).select{|p| p != "" }

      dir_indx_before_11 = dirs_to_look.find_index{|d| all_files_exist( d, openssl_before_11 ) }
      dir_indx_after_11 = dirs_to_look.find_index{|d| all_files_exist( d, openssl_after_11 ) }
    end

    if dir_indx_before_11 and dir_indx_after_11
      if dir_indx_before_11 < dir_indx_after_11
        openssl_before_11
      else
        openssl_after_11
      end
    elsif dir_indx_before_11
      openssl_before_11
    elsif dir_indx_after_11
      openssl_after_11
    else
      []
    end
  end

  def self.get_libs_vc
    find_first_openssl_libs(
      [ 'libeay32.lib', 'ssleay32.lib' ],
      [ 'libssl.lib', 'libcrypto.lib' ] )
  end

  def self.get_libs_linux
    [ 'ssl', 'crypto' ]
  end

  def self.get_libs_mingw
    get_libs_linux + [ 'gdi32' ]
  end


  def self.get_libs( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_mingw
        end
    else
      get_libs_linux
    end
  end

  # Returns the name from environment variable OPENSSL_PRJ_FILE or
  # name 'local-openssl.rb' if OPENSSL_PRJ_FILE is not set.
  #
  # Note that this file can be nonexistent. Because of that File.exist?
  # must be used to check presence of this file.
  def self.try_to_get_custom_file
    custom_local_openssl_prj = ENV[ "OPENSSL_PRJ_FILE" ]
    custom_local_openssl_prj = 'local-openssl.rb' unless custom_local_openssl_prj
    custom_local_openssl_prj
  end

  def self.get_lib_dirs( toolset )
    libs = []
    IO.popen( "#{toolset.cpp_compiler_name} -print-search-dirs 2>&1",
              :err => [:child, :out] ) do |io|
      io.each_line do |line|
        if /^libraries: =(?<libstr>.*)/ =~ line
          splitter = ':'
          if 'mswin' == toolset.tag( 'target_os' )
            splitter = ';'
          end
          libs += libstr.split( splitter )
        end
      end
    end
    libs
  end

  def self.check_libs_available( toolset, libs )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          true
        elsif [ 'gcc', 'clang' ].include? toolset.name
          get_lib_dirs( toolset ).find_index{|d| all_files_exist( d, libs ) }
        end
    else
      get_lib_dirs( toolset ).find_index{|d| all_files_exist( d, libs ) }
    end
  end

  def self.has_openssl( toolset )
    custom_prj = try_to_get_custom_file
    if File.exist?(custom_prj)
      true
    else
      libraries = get_libs( toolset )
      if !libraries.empty?
        check_libs_available( toolset, libraries )
      else
        false
      end
    end
  end

end #module RestinioOpenSSLFind

# vim:ts=2:sts=2:sw=2:expandtab

