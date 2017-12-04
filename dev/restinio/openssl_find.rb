require File.join( File.dirname(__FILE__), 'lib_finder.rb' )

module RestinioOpenSSLFind

  def self.find_first_openssl_libs( openssl_before_11, openssl_after_11 )

    lib_dirs = ENV[ "LIB" ]
    if lib_dirs

      dirs_to_look = lib_dirs.split( ';' ).select{|p| p != "" }

      dir_indx_before_11 = dirs_to_look.find_index{|d| RestinioLibFinder.all_files_exist( d, openssl_before_11 ) }
      dir_indx_after_11 = dirs_to_look.find_index{|d| RestinioLibFinder.all_files_exist( d, openssl_after_11 ) }
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

  def self.get_libs_names_linux
    [ 'libssl.so', 'libcrypto.so' ]
  end

  def self.get_libs_mingw
    [ 'ssl', 'crypto', 'gdi32' ]
  end

  def self.get_libs_names_mingw
    [ 'libssl.a', 'libcrypto.a', 'libgdi32.a' ]
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

  def self.get_libs_names( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_names_mingw
        end
    else
      get_libs_names_linux
    end
  end

  def self.try_to_get_custom_file
    RestinioLibFinder.try_to_get_custom_file( 'OPENSSL_PRJ_FILE', 'local-openssl.rb' )
  end

  def self.has_openssl( toolset )
    custom_prj = try_to_get_custom_file
    if File.exist?(custom_prj)
      true
    else
      libraries = get_libs_names( toolset )
      if !libraries.empty?
        RestinioLibFinder.check_libs_available( toolset, libraries )
      else
        false
      end
    end
  end

end #module RestinioOpenSSLFind

# vim:ts=2:sts=2:sw=2:expandtab

